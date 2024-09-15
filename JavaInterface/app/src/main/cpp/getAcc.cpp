#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>
#include <jni.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "sensor-native", __VA_ARGS__))
#define SERVER_IP "192.168.1.15"
#define SERVER_PORT 8080

static ASensorManager* sensorManager;
static const ASensor* accelerometer;
static const ASensor* gyroscope;
static const ASensor* magnetometer;
static ASensorEventQueue* sensorEventQueue;
static JavaVM* javaVM = nullptr;
static jobject globalMainActivityObj = nullptr;
static int sockfd; // Socket file descriptor

// Function to send data (tagged by sensor_type) over the socket
void sendDataToServer(const char* sensorType, float x, float y, float z)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s,%.6f,%.6f,%.6f\n", sensorType, x, y, z);

    // Send the data through the socket
    if (send(sockfd, buffer, strlen(buffer), 0) < 0)
    {
        LOGI("Failed to send data to server");
    }
    else
        LOGI("Data sent to server: %s", buffer);
}

static int get_sensor_data(int fd, int events, void* data)
{
    ASensorEvent event;
    JNIEnv* env;
    javaVM->AttachCurrentThread(&env, nullptr);  // Attach the current thread to the JVM

    // Get the Java class and method to call
    jclass activityClass = env->GetObjectClass(globalMainActivityObj);
    jmethodID updateAccelerometerData = env->GetMethodID(activityClass, "updateAccelerometerData", "(FFF)V");
    jmethodID updateGyroscopeData = env->GetMethodID(activityClass, "updateGyroscopeData", "(FFF)V");
    jmethodID updateMagnetometerData = env->GetMethodID(activityClass, "updateMagnetometerData", "(FFF)V");

    // Loop through the sensor events and handle them
    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        if (event.type == ASENSOR_TYPE_ACCELEROMETER)
        {
            LOGI("Acc: x = %f, y = %f, z = %f", event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Send data to the server
            sendDataToServer("Acc", event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Call the Java method to update the accelerometer UI
            env->CallVoidMethod(globalMainActivityObj, updateAccelerometerData, event.acceleration.x, event.acceleration.y, event.acceleration.z);
        }
        else if (event.type == ASENSOR_TYPE_GYROSCOPE)
        {
            LOGI("Gyro: x = %f, y = %f, z = %f", event.vector.x, event.vector.y, event.vector.z);

            // Send data to the server
            sendDataToServer("Gyro", event.vector.x, event.vector.y, event.vector.z);

            // Call the Java method to update the gyroscope UI
            env->CallVoidMethod(globalMainActivityObj, updateGyroscopeData, event.vector.x, event.vector.y, event.vector.z);
        }
        else if (event.type == ASENSOR_TYPE_MAGNETIC_FIELD)
        {
            LOGI("Mag: x = %f, y = %f, z = %f", event.magnetic.x, event.magnetic.y, event.magnetic.z);

            // Send data to the server
            sendDataToServer("Mag", event.magnetic.x, event.magnetic.y, event.magnetic.z);

            // Call the Java method to update the magnetometer UI
            env->CallVoidMethod(globalMainActivityObj, updateMagnetometerData, event.magnetic.x, event.magnetic.y, event.magnetic.z);
        }
    }

    // **Note**: We are not detaching the thread here anymore. We'll handle it later if needed.

    return 1;  // Return 1 to indicate success
}

// Function to set up the socket connection to the server
void setupSocket() {
    struct sockaddr_in server_addr;
    const char* password = "Sensor-Stream2024!"; // Password to match with server

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOGI("Failed to create socket");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        LOGI("Failed to connect to server");
        close(sockfd);
    }

    // Send the password to the server
    if (send(sockfd, password, strlen(password), 0) < 0)
    {
        LOGI("Failed to send password");
        close(sockfd);
        return;
    }

    LOGI("Connected to server at %s:%d", SERVER_IP, SERVER_PORT);
}


// Native method to start accelerometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startAccelerometer(JNIEnv* env, jobject obj)
{
    // Store the Java VM and MainActivity reference globally for later use
    env->GetJavaVM(&javaVM);
    globalMainActivityObj = env->NewGlobalRef(obj);

    // Set up the socket connection to the server
    setupSocket();

    // Get the sensor manager instance (with fallback for older devices)
    sensorManager = ASensorManager_getInstanceForPackage("");  // Updated to avoid deprecation warning
    if (!sensorManager)
    {
        sensorManager = ASensorManager_getInstance();  // Fallback for older devices.
    }

    // Get the accelerometer sensor
    accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), 0, get_sensor_data, nullptr);

    // Enable the accelerometer sensor and set the event rate to 50Hz (20ms intervals)
    ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
    ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, 20000);  // 50Hz
}

// Native method to stop accelerometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_stopAccelerometer(JNIEnv* env, jobject obj)
{
    ASensorEventQueue_disableSensor(sensorEventQueue, accelerometer);
    close(sockfd); // Close the socket connection
    LOGI("Socket closed");
}

// Native method to start gyroscope
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startGyroscope(JNIEnv* env, jobject obj)
{
    // Get the default gyroscope sensor
    gyroscope = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_GYROSCOPE);
    // Enable the accelerometer sensor and set the event rate to 50Hz (20ms intervals)
    ASensorEventQueue_enableSensor(sensorEventQueue, gyroscope);
    ASensorEventQueue_setEventRate(sensorEventQueue, gyroscope, 20000);  // 50Hz
}

// Native method to stop gyroscope
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_stopGyroscope(JNIEnv* env, jobject obj)
{
    ASensorEventQueue_disableSensor(sensorEventQueue, gyroscope);
}

// Native method to start magnetometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startMagnetometer(JNIEnv* env, jobject obj)
{
    // Get the default gyroscope sensor
    magnetometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_MAGNETIC_FIELD);
    // Enable the accelerometer sensor and set the event rate to 50Hz (20ms intervals)
    ASensorEventQueue_enableSensor(sensorEventQueue, magnetometer);
    ASensorEventQueue_setEventRate(sensorEventQueue, magnetometer, 20000);  // 50Hz
}

// Native method to stop magnetometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_stopMagnetometer(JNIEnv* env, jobject obj)
{
    ASensorEventQueue_disableSensor(sensorEventQueue, magnetometer);
}