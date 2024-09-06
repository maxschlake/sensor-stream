#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>
#include <jni.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "sensor-native", __VA_ARGS__))

static ASensorManager* sensorManager;
static const ASensor* accelerometer;
static ASensorEventQueue* sensorEventQueue;

static int get_sensor_data(int fd, int events, void* data)
{
    ASensorEvent event;
    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        if (event.type == ASENSOR_TYPE_ACCELEROMETER)
        {
            LOGI("Acc: x = %f, y = %f, z = %f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
        }
    }
    return 1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_example_javainterface_MainActivity_startSensor(JNIEnv* env, jobject)
{
    sensorManager = ASensorManager_getInstance();
    accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), 0, get_sensor_data, nullptr);

    ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
    ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, 1000000); // 1-second intervals
}
