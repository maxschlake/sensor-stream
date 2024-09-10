#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>
#include <jni.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "sensor-native", __VA_ARGS__))

static ASensorManager* sensorManager;
static const ASensor* accelerometer;
static ASensorEventQueue* sensorEventQueue;
static JavaVM* javaVM = nullptr;
static jobject globalMainActivityObj = nullptr;

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

            // Call the Java method to update the UI with new accelerometer data
            env->CallVoidMethod(globalMainActivityObj, updateSensorData, event.acceleration.x, event.acceleration.y, event.acceleration.z);
        }
    }

    // **Note**: We are not detaching the thread here anymore. We'll handle it later if needed.

    return 1;  // Return 1 to indicate success
}

extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startSensor(JNIEnv* env, jobject obj)
{
    // Store the Java VM and MainActivity reference globally for later use
    env->GetJavaVM(&javaVM);
    globalMainActivityObj = env->NewGlobalRef(obj);

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
