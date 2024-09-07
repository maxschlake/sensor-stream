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
    javaVM->AttachCurrentThread(&env, nullptr);

    jclass activityClass = env->GetObjectClass(globalMainActivityObj);
    jmethodID updateSensorData = env->GetMethodID(activityClass, "updateSensorData", "(FFF)V");

    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        if (event.type == ASENSOR_TYPE_ACCELEROMETER)
        {
            LOGI("Acc: x = %f, y = %f, z = %f", event.acceleration.x, event.acceleration.y, event.acceleration.z);

            // Call the Java method to update the UI
            env->CallVoidMethod(globalMainActivityObj, updateSensorData, event.acceleration.x, event.acceleration.y, event.acceleration.z);
        }
    }
    javaVM->DetachCurrentThread();
    return 1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startSensor(JNIEnv* env, jobject obj)
{
    // Store the Java VM and MainActivity reference
    env->GetJavaVM(&javaVM);
    globalMainActivityObj = env->NewGlobalRef(obj);

    sensorManager = ASensorManager_getInstanceForPackage("");  // Updated to avoid deprecation warning

    if (!sensorManager) {
        sensorManager = ASensorManager_getInstance();  // Fallback for older devices.
    }

    accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), 0, get_sensor_data, nullptr);

    // Set the event rate to 50Hz (20 milliseconds) by dividing 1000000 / 50
    ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
    ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, 1000000); // 1-second intervals
}

