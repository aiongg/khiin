//
// Created by aiong on 3/20/2023.
//


#include <jni.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "engine/Engine.h"
#include "proto/proto/command.pb.h"

using namespace khiin::engine;

extern "C"
JNIEXPORT jlong JNICALL
Java_be_chiahpa_khiin_EngineManager_load(
        JNIEnv *env,
        jobject thiz,
        jstring dbFileName
) {
    jboolean is_copy;
    const char *c_str = env->GetStringUTFChars(dbFileName, &is_copy);
    std::string db_file_name{c_str};
    auto engine = Engine::Create(db_file_name);
    return (jlong) engine.release();
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_be_chiahpa_khiin_EngineManager_sendCommand(
        JNIEnv *env,
        jobject thiz,
        jlong engine_ptr,
        jbyteArray cmd_bytes
) {
    khiin::proto::Command cmd;

    jbyte *bufferElems = env->GetByteArrayElements(cmd_bytes, nullptr);
    int len = env->GetArrayLength(cmd_bytes);
    try {
        cmd.ParseFromArray(reinterpret_cast<unsigned char *>(bufferElems), len);
    } catch (...) {}
    env->ReleaseByteArrayElements(cmd_bytes, bufferElems, JNI_ABORT);

    auto *engine = reinterpret_cast<Engine *>(engine_ptr);
    engine->SendCommand(&cmd);

    auto bytes = cmd.SerializeAsString();
    const char* buffer = bytes.data();
    auto size = (int) bytes.size();
//    auto size = cmd.ByteSize();
//    auto *buffer = new char[size];
//    cmd.SerializeToArray(buffer, size);
    auto ret = env->NewByteArray(size);
    env->SetByteArrayRegion(ret, 0, size, (jbyte *) buffer);

    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_be_chiahpa_khiin_EngineManager_shutdown(
        JNIEnv *env,
        jobject thiz,
        jlong engine_ptr
) {
    delete reinterpret_cast<Engine*>(engine_ptr);
}
