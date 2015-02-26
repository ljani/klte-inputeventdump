#include <android/configuration.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>

#define APPNAME "klte-inputeventdump"

#define LOG(level, msg, ...) __android_log_print(level, APPNAME, msg, ## __VA_ARGS__);
#define LOG_WARN(msg, ...) LOG(ANDROID_LOG_WARN, msg, ## __VA_ARGS__)

AInputQueue *inputQueue{};

void readEvent(AInputEvent *event) {
	auto type = AInputEvent_getType(event);
	auto source = AInputEvent_getSource(event);
	LOG_WARN("Type: %#010x Source: %#010x", type, source);
}

void readQueue(AInputQueue *queue) {
	AInputEvent* event = nullptr;
	while(AInputQueue_getEvent(inputQueue, &event) >= 0) {
		readEvent(event);
	}
}

static void registerCallbacks(ANativeActivity *activity) {
	activity->callbacks->onInputQueueCreated = [](ANativeActivity *, AInputQueue *queue) {
        inputQueue = queue;
        AInputQueue_attachLooper(queue, ALooper_forThread(), ALOOPER_POLL_CALLBACK, [](int, int, void *data) {
            readQueue((AInputQueue *)data);
            return 1;
        }, queue);
    };
	activity->callbacks->onInputQueueDestroyed = [](ANativeActivity *, AInputQueue *queue) {
        inputQueue = nullptr;
        AInputQueue_detachLooper(queue);
    };
}

#define EXPORT __attribute__((visibility("default"), externally_visible))
extern "C" void EXPORT ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize) {
    LOG_WARN("ANativeActivity_onCreate");
    registerCallbacks(activity);
}
