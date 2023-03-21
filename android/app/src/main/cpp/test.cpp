#include <jni.h>
#include <memory>
#include <string>

#include "engine/Engine.h"

using namespace khiin::engine;

extern "C"
JNIEXPORT jstring JNICALL
Java_be_chiahpa_khiin_SettingsActivity_stringFromJNI(
        JNIEnv *env,
        jobject
) {
    std::unique_ptr<Engine> engine = Engine::Create();

    std::string hello = "Hello";
    return env->NewStringUTF(hello.c_str());
}
