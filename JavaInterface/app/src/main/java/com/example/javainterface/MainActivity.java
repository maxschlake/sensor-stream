package com.example.javainterface;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

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
    }

    // This method will be called from native code to update the sensor data
    public void updateSensorData(float x, float y, float z) {
        // Update the TextViews with new accelerometer values
        runOnUiThread(() -> {
            xTextView.setText(String.format("X: %.2f", x));
            yTextView.setText(String.format("Y: %.2f", y));
            zTextView.setText(String.format("Z: %.2f", z));
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopSensor();
    }
}