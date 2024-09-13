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

    // Declare the native method to start the sensor
    public native void startSensor();
    public native void stopSensor();

    // UI elements to display the accelerometer date
    private TextView xTextView, yTextView, zTextView;

    // Declare a handler to stop the sensor after a fixed period
    private Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize TextViews
        xTextView = findViewById(R.id.x_value);
        yTextView = findViewById(R.id.y_value);
        zTextView = findViewById(R.id.z_value);

        // Start the sensor from the native side
        startSensor();

        // Stop the sensor after 10 seconds (10000 milliseconds)
        handler.postDelayed(() -> stopSensor(), 10000);
    }

    // This method will be called from native code to update the sensor data
    public void updateSensorData(float x, float y, float z) {
        // Update the TextViews with new accelerometer values
        runOnUiThread(() -> {
            xTextView.setText(String.format("xAcc: %.6f", x));
            yTextView.setText(String.format("yAcc: %.6f", y));
            zTextView.setText(String.format("zAcc: %.6f", z));
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopSensor();
    }
}