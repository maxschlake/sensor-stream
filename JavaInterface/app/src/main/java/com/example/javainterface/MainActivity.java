package com.example.javainterface;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.os.Handler;

public class MainActivity extends AppCompatActivity {
    // Used to load the 'getAcc' library on application startup
    static {
        System.loadLibrary("getAcc");
    }

    // Native methods for the sensors
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

        // Start the sensors from the native side
        startAccelerometer();
        startGyroscope();
        startMagnetometer();

        // Stop the sensors after 10 seconds (10000 milliseconds)
        handler.postDelayed(() -> {
            stopAccelerometer();
            stopGyroscope();
            stopMagnetometer();
        }, 10000);
    }

    // Method to update the accelerometer data
    public void updateAccelerometerData(float x, float y, float z) {
        runOnUiThread(() -> {
            xAccTextView.setText(String.format("xAcc: %.6f", x));
            yAccTextView.setText(String.format("yAcc: %.6f", y));
            zAccTextView.setText(String.format("zAcc: %.6f", z));
        });
    }

    // Method to update the gyroscope data
    public void updateGyroscopeData(float x, float y, float z) {
        runOnUiThread(() -> {
            xGyroTextView.setText(String.format("xGyro: %.6f", x));
            yGyroTextView.setText(String.format("yGyro: %.6f", y));
            zGyroTextView.setText(String.format("zGyro: %.6f", z));
        });
    }

    // Method to update the gyroscope data
    public void updateMagnetometerData(float x, float y, float z) {
        runOnUiThread(() -> {
            xMagTextView.setText(String.format("xMag: %.6f", x));
            yMagTextView.setText(String.format("yMag: %.6f", y));
            zMagTextView.setText(String.format("zMag: %.6f", z));
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopAccelerometer();
        stopGyroscope();
        stopMagnetometer();
    }
}