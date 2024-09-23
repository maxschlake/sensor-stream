#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>
#include <jni.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>

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

// Function to collect and display sensor data without needing the socket
void displaySensorDataLocally(JNIEnv* env, const char* sensorType, float x, float y, float z)
{
    jclass activityClass = env->GetObjectClass(globalMainActivityObj);

    if (strcmp(sensorType, "Acc") == 0)
    {
        jmethodID updateAccelerometerData = env->GetMethodID(activityClass, "updateAccelerometerData", "(FFF)V");
        env->CallVoidMethod(globalMainActivityObj, updateAccelerometerData, x, y, z);
    }
    else if (strcmp(sensorType, "Gyro") == 0)
    {
        jmethodID updateGyroscopeData = env->GetMethodID(activityClass, "updateGyroscopeData", "(FFF)V");
        env->CallVoidMethod(globalMainActivityObj, updateGyroscopeData, x, y, z);
    }
    else if (strcmp(sensorType, "Mag") == 0)
    {
        jmethodID updateMagnetometerData = env->GetMethodID(activityClass, "updateMagnetometerData", "(FFF)V");
        env->CallVoidMethod(globalMainActivityObj, updateMagnetometerData, x, y, z);
    }
}

// Function to send data to the server once the password is validated
void sendSensorDataToServer(const char* sensorType, float x, float y, float z)
{
    if (sockfd < 0)
    {
        LOGI("Socket not open; skipping data transmission");
        return;
    }

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

// Global flag to indicate if the server is ready
static bool startSending = false;

// Native method to set the server readiness
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startSending(JNIEnv* env, jobject obj, jboolean validated)
{
    startSending = validated; // Update the global isServerReady flag
    if (!startSending)
    {
        close(sockfd);
    }
}

static int handleSensorEvents(int fd, int events, void* data)
{
    ASensorEvent event;
    JNIEnv* env;
    javaVM->AttachCurrentThread(&env, nullptr);  // Attach the current thread to the JVM

    // Loop through the sensor events and handle them
    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        if (event.type == ASENSOR_TYPE_ACCELEROMETER)
        {
            LOGI("Acc: x = %f, y = %f, z = %f", event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Display data on UI
            displaySensorDataLocally(env, "Acc", event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Send in case the password is validated
            if (startSending)
            {
                sendSensorDataToServer("Acc", event.acceleration.x, event.acceleration.y, event.acceleration.z);
            }
        }
        else if (event.type == ASENSOR_TYPE_GYROSCOPE)
        {
            LOGI("Gyro: x = %f, y = %f, z = %f", event.vector.x, event.vector.y, event.vector.z);

            // Display data on UI
            displaySensorDataLocally(env, "Gyro", event.vector.x, event.vector.y, event.vector.z);

            // Send in case the password is validated
            if (startSending)
            {
                sendSensorDataToServer("Gyro", event.acceleration.x, event.acceleration.y, event.acceleration.z);
            }
        }
        else if (event.type == ASENSOR_TYPE_MAGNETIC_FIELD)
        {
            LOGI("Mag: x = %f, y = %f, z = %f", event.magnetic.x, event.magnetic.y, event.magnetic.z);

            // Display data on UI
            displaySensorDataLocally(env, "Mag", event.magnetic.x, event.magnetic.y, event.magnetic.z);

            // Send in case the password is validated
            if (startSending)
            {
                sendSensorDataToServer("Mag", event.acceleration.x, event.acceleration.y, event.acceleration.z);
            }
        }
    }

    return 1;  // Return 1 to indicate success
}

// Function to set up the socket connection to the server and validate the password
extern "C"
JNIEXPORT jboolean JNICALL Java_com_example_javainterface_MainActivity_connectToServerAndValidatePassword(JNIEnv* env, jobject obj, jstring jPassword)
{
    const char* password = env->GetStringUTFChars(jPassword, nullptr);
    struct sockaddr_in server_addr;

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOGI("Failed to create socket");
        return JNI_FALSE;
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        LOGI("Failed to connect to the server");
        close(sockfd);
        return JNI_FALSE;
    }
    LOGI("Connected to the server at %s:%d", SERVER_IP, SERVER_PORT);

    // Send the password to the server for validation
    if (send(sockfd, password, strlen(password), 0) < 0)
    {
        LOGI("Failed to send password");
        close(sockfd);
        return JNI_FALSE;
    }
    LOGI("Password sent to server. Waiting for validation...");

    // Receive the server's response
    char buffer[256];
    ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        LOGI("Failed to receive response");
        close(sockfd);
        return JNI_FALSE;
    }

    buffer[bytes_received] = '\0';  // Null-terminate the response

    // Interpret the server's response (assuming the server sends "AUTHORIZED" for a correct response)
    if (strncmp(buffer, "AUTHORIZED", strlen("AUTHORIZED")) == 0)
    {
        LOGI("Password validated by server");
        return JNI_TRUE;
    }
    else
    {
        LOGI("Invalid password, server rejected");
        close(sockfd);
        return JNI_FALSE;
    }
}

// Global initialization function for the sensor manager
void initializeSensorManager(JNIEnv* env, jobject obj)
{
    // Only initialize once
    if (javaVM == nullptr)
    {
        // Store the Java VM and MainActivity reference globally for later use
        env->GetJavaVM(&javaVM);
        globalMainActivityObj = env->NewGlobalRef(obj);

        // Get the sensor manager instance (with fallback for older devices)
        sensorManager = ASensorManager_getInstanceForPackage("");  // Updated to avoid deprecation warning
        if (!sensorManager)
        {
            sensorManager = ASensorManager_getInstance();  // Fallback for older devices.
        }
        sensorEventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), 0, handleSensorEvents, nullptr);
    }
}

// Native method to start accelerometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startAccelerometer(JNIEnv* env, jobject obj)
{
    // Initialize common resources if not already initialized
    initializeSensorManager(env, obj);

    // Get the default accelerometer sensor
    accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);

    // Enable the accelerometer sensor and set the event rate to 50Hz (20ms intervals)
    ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
    ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, 20000);  // 50Hz
}

// Native method to stop accelerometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_stopAccelerometer(JNIEnv* env, jobject obj)
{
    if (sensorEventQueue && accelerometer)
    {
        ASensorEventQueue_disableSensor(sensorEventQueue, accelerometer);
    }
    close(sockfd); // Consistently stop the socket after stopping the accelerometer
}

// Native method to start gyroscope
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startGyroscope(JNIEnv* env, jobject obj)
{
    // Initialize common resources if not already initialized
    initializeSensorManager(env, obj);

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
    if (sensorEventQueue && gyroscope)
    {
        ASensorEventQueue_disableSensor(sensorEventQueue, gyroscope);
    }
    close(sockfd); // Consistently stop the socket after stopping the gyroscope
}

// Native method to start magnetometer
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startMagnetometer(JNIEnv* env, jobject obj)
{
    // Initialize common resources if not already initialized
    initializeSensorManager(env, obj);

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
    if (sensorEventQueue && magnetometer)
    {
        ASensorEventQueue_disableSensor(sensorEventQueue, magnetometer);
    }
    close(sockfd); // Consistently stop the socket after stopping the magnetometer
}