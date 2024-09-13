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
static ASensorEventQueue* sensorEventQueue;
static JavaVM* javaVM = nullptr;
static jobject globalMainActivityObj = nullptr;
static int sockfd; // Socket file descriptor

// Function to send accelerometer data over the socket
void sendDataToServer(float x, float y, float z)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%.6f,%.6f,%.6f\n", x, y, z);

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
    jmethodID updateSensorData = env->GetMethodID(activityClass, "updateSensorData", "(FFF)V");

    // Loop through the sensor events and handle them
    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        if (event.type == ASENSOR_TYPE_ACCELEROMETER)
        {
            LOGI("Acc: x = %f, y = %f, z = %f", event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Send data to the server
            sendDataToServer(event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Call the Java method to update the UI
            env->CallVoidMethod(globalMainActivityObj, updateSensorData, event.acceleration.x, event.acceleration.y, event.acceleration.z);
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

extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startSensor(JNIEnv* env, jobject obj)
{
    // Store the Java VM and MainActivity reference globally for later use
    env->GetJavaVM(&javaVM);
    globalMainActivityObj = env->NewGlobalRef(obj);

    // Set up the socket connection to the server
    setupSocket();

    // Get the sensor manager instance (with fallback for older devices)
    sensorManager = ASensorManager_getInstanceForPackage("");  // Updated to avoid deprecation warning
    if (!sensorManager) {
        sensorManager = ASensorManager_getInstance();  // Fallback for older devices.
    }

    // Get the default accelerometer sensor
    accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);

    // Create the event queue for the accelerometer sensor
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), 0, get_sensor_data, nullptr);
    // Enable the accelerometer sensor and set the event rate to 50Hz (20ms intervals)
    ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);

    // Set event rate to 50Hz (20 milliseconds = 20000 microseconds)
    ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, 20000);  // 50Hz
}

// Clean up function to close the socket when done
extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_stopSensor(JNIEnv* env, jobject obj)
{
    ASensorEventQueue_disableSensor(sensorEventQueue, accelerometer);
    close(sockfd); // Close the socket connection
    LOGI("Socket closed");
}