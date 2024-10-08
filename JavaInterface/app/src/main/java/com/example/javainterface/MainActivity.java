package com.example.javainterface;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;
import android.widget.Button;
import android.os.Handler;
import android.app.AlertDialog;
import android.widget.EditText;
import android.content.DialogInterface;
import android.widget.Toast;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;

import android.content.Intent;
import android.net.VpnService;  // Import VpnService

public class MainActivity extends AppCompatActivity {
    // Used to load the 'getSensorData' library on application startup
    static {
        System.loadLibrary("getSensorData");
    }
    private static final int VPN_REQUEST_CODE = 100;

    // Native methods
    public native boolean connectToServerAndValidatePassword(String password); // For server connection
    public native void startSending(boolean validated);

    public native void startAccelerometer(); // For accelerometer
    public native void stopAccelerometer();

    public native void startGyroscope(); // For gyroscope
    public native void stopGyroscope();

    public native void startMagnetometer(); // For magnetometer
    public native void stopMagnetometer();

    // UI elements to display the sensor data
    private TextView xAccTextView, yAccTextView, zAccTextView;
    private TextView xGyroTextView, yGyroTextView, zGyroTextView;
    private TextView xMagTextView, yMagTextView, zMagTextView;

    private Button sendToServerButton;

    // Declare a handler to stop the sensor after a fixed period
    private Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize accelerometer TextViews
        xAccTextView = findViewById(R.id.x_acc_value);
        yAccTextView = findViewById(R.id.y_acc_value);
        zAccTextView = findViewById(R.id.z_acc_value);

        // Initialize gyroscope TextViews
        xGyroTextView = findViewById(R.id.x_gyro_value);
        yGyroTextView = findViewById(R.id.y_gyro_value);
        zGyroTextView = findViewById(R.id.z_gyro_value);

        // Initialize magnetometer TextViews
        xMagTextView = findViewById(R.id.x_mag_value);
        yMagTextView = findViewById(R.id.y_mag_value);
        zMagTextView = findViewById(R.id.z_mag_value);

        // Initialize the button to send data fot the server
        sendToServerButton = findViewById(R.id.sendToServerButton);

        // Set OnClickListener on the button
        sendToServerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                startVpn();
            }
        });

        // Start displaying sensor data on app startup (without server connection)
        startAccelerometer();
        startGyroscope();
        startMagnetometer();
    }

    // VPN-related method
    private void startVpn()
    {
        Intent vpnIntent = VpnService.prepare(MainActivity.this);
        if (vpnIntent != null)
        {
            startActivityForResult(vpnIntent, VPN_REQUEST_CODE);
        }
        else
        {
            onActivityResult(VPN_REQUEST_CODE, RESULT_OK, null);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if (requestCode == VPN_REQUEST_CODE && resultCode == RESULT_OK)
        {
            // VPN is ready, now start the VPN service
            Intent intent = new Intent(MainActivity.this, MyVpnService.class);
            startService(intent);

            // Proceed to password validation and data transmission
            promptForDataTransmission();
        }
        else if (resultCode != RESULT_OK)
        {
            // Notify the user if VPN connection fails
            Toast.makeText(MainActivity.this, "VPN permission denied", Toast.LENGTH_SHORT).show();
        }
    }

    // Prompt the user if they want to send data to the server
    private void promptForDataTransmission()
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);

        // Create a TextView for the title and set its properties
        TextView title = new TextView(this);
        title.setText(getString(R.string.enter_seconds));
        title.setPadding(10, 50, 0, 0);
        title.setTextSize(20);
        title.setTextColor(getResources().getColor(android.R.color.black));

        // Set up the custom title
        builder.setCustomTitle(title);

        // Set up the input (how many seconds of data to send)
        EditText input = new EditText(this);
        input.setHint("Seconds");

        // Restrict input to numbers only
        input.setInputType(InputType.TYPE_CLASS_NUMBER);

        builder.setView(input);

        // Set up the buttons
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                String seconds = input.getText().toString().trim();

                // Hide the keyboard after the user input
                hideKeyboard(input);

                // Ensure the input is not empty
                if (seconds.isEmpty())
                {
                    Toast.makeText(MainActivity.this, "Please enter a valid number of seconds", Toast.LENGTH_SHORT).show();
                    return;
                }
                else
                {
                    try
                    {
                        int secondsValue = Integer.parseInt(seconds);
                        // After seconds are entered, ask for the password
                        promptForPassword(secondsValue);
                    }
                    catch (NumberFormatException e)
                    {
                        Toast.makeText(MainActivity.this, "Invalid input - please enter a valid number", Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });
        builder.setNegativeButton("CANCEL", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                dialog.cancel();
            }
        });
        builder.show();
    }

    // Prompt the user to enter the password before proceeding with data transmission
    private void promptForPassword(int seconds)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);

        // Create a TextView for the title and set its properties
        TextView title = new TextView(this);
        title.setText(getString(R.string.enter_password));
        title.setPadding(10, 50, 0, 0);
        title.setTextSize(20);
        title.setTextColor(getResources().getColor(android.R.color.black));

        // Set up the custom title
        builder.setCustomTitle(title);

        // Set up the input (password)
        EditText passwordInput = new EditText(this);
        passwordInput.setHint("Password");

        // Set input type to hide text input with '*' (password style)
        passwordInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        builder.setView(passwordInput);

        // Set up the buttons
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                String password = passwordInput.getText().toString();

                // Hide the keyboard after the user input
                hideKeyboard(passwordInput);

                // Run the network operation in a background thread
                new Thread(() ->
                {
                    // Try to connect to the server and validate the password
                    if (connectToServerAndValidatePassword(password)) {
                        // Update the UI on the main thread
                        runOnUiThread(() ->
                        {
                            Toast.makeText(MainActivity.this, "Password correct - sending data for " + seconds + "seconds", Toast.LENGTH_SHORT).show();
                            startSending(true); // Send data

                            // Stop sensor collection after 'seconds' seconds
                            handler.postDelayed(() ->
                            {
                                startSending(false);
                                Toast.makeText(MainActivity.this, "Data transmission ended", Toast.LENGTH_SHORT).show();
                            }, seconds * 1000);
                        });
                    } else {
                        runOnUiThread(() ->
                        {
                            Toast.makeText(MainActivity.this, "Invalid password", Toast.LENGTH_SHORT).show();
                            startSending(false); // Do not send data
                        });
                    }
                }).start();
            }
        });
        builder.setNegativeButton("CANCEL", new DialogInterface.OnClickListener()
        {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                dialog.cancel();
            }
        });
        builder.show();
    }

    private void hideKeyboard(View view)
    {
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }

    // Method to update the accelerometer data
    public void updateAccelerometerData(float x, float y, float z)
    {
        runOnUiThread(() ->
        {
            xAccTextView.setText(String.format("%.6f", x));
            yAccTextView.setText(String.format("%.6f", y));
            zAccTextView.setText(String.format("%.6f", z));
        });
    }

    // Method to update the gyroscope data
    public void updateGyroscopeData(float x, float y, float z)
    {
        runOnUiThread(() ->
        {
            xGyroTextView.setText(String.format("%.6f", x));
            yGyroTextView.setText(String.format("%.6f", y));
            zGyroTextView.setText(String.format("%.6f", z));
        });
    }

    // Method to update the gyroscope data
    public void updateMagnetometerData(float x, float y, float z)
    {
            runOnUiThread(() ->
            {
                xMagTextView.setText(String.format("%.6f", x));
                yMagTextView.setText(String.format("%.6f", y));
                zMagTextView.setText(String.format("%.6f", z));
            });
    }

    // Method to stop all sensors
    private void stopAllSensors()
    {
        stopAccelerometer();
        stopGyroscope();
        stopMagnetometer();
        Toast.makeText(MainActivity.this, "Data collection stopped", Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        stopAllSensors();
    }
}