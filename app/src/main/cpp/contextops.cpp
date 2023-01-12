//
// Created by maks on 15.10.2022.
//
#include <cstring>
#include <android/log.h>
#include "main.h"
#include "contextops.h"

#include <android/asset_manager_jni.h>
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static bool context_operable = false;
static bool assets_operable = false;
static jclass class_ContextOps;
static jmethodID method_getClipboard;
static jmethodID method_setClipboard;
static jmethodID method_getAssetManager;
void contextops_initIDs(JNIEnv* env) {
    class_ContextOps = LoadClass(env, "git.artdeell.aw4c.ContextOps");
    if(!class_ContextOps) return;
    class_ContextOps = (jclass) env->NewGlobalRef(class_ContextOps);
    method_getClipboard = env->GetStaticMethodID(class_ContextOps, "getClipboard", "()Ljava/lang/String;");
    method_setClipboard = env->GetStaticMethodID(class_ContextOps, "setClipboard", "(Ljava/lang/String;)V");
    method_getAssetManager = env->GetStaticMethodID(class_ContextOps, "getAssetManager", "()Landroid/content/res/AssetManager;");
    if(!method_setClipboard || !method_getClipboard || !method_getAssetManager) return;
    context_operable = env->CallStaticBooleanMethod(class_ContextOps, env->GetStaticMethodID(class_ContextOps, "init", "()Z"));
    assets_operable = env->CallStaticBooleanMethod(class_ContextOps, env->GetStaticMethodID(class_ContextOps, "hasAssets", "()Z"));
}
static char* cbuf;
static size_t coff;
static size_t clen;
void contextops_getClipboard0(JNIEnv* env) {
    auto clipstr = (jstring) env->CallStaticObjectMethod(class_ContextOps, method_getClipboard);
    if(clipstr == nullptr) return;
    const char* chars = env->GetStringUTFChars(clipstr, nullptr);
    strncpy(cbuf+coff, chars, MIN(env->GetStringUTFLength(clipstr), clen - coff));
    env->ReleaseStringUTFChars(clipstr, chars);
}
void contextops_setClipboard0(JNIEnv *env) {
    env->CallStaticVoidMethod(class_ContextOps,method_setClipboard, env->NewStringUTF(cbuf));
}
void contextops_getClipboard(char* buf, size_t off, size_t len) {
    if(context_operable) {
        cbuf = buf;
        coff = off;
        clen = len;
        JNIWrapper(&contextops_getClipboard0);
    }
}
void contextops_setClipboard(char* buf) {
    if(context_operable) {
        cbuf = buf;
        JNIWrapper(&contextops_setClipboard0);
    }
}
bool contextops_available() {
    return context_operable;
}
bool contextops_assets_available() {
    return assets_operable;
}

void* contextops_get_assets0(JNIEnv* env) {
    jobject assets = env->CallStaticObjectMethod(class_ContextOps, method_getAssetManager);
    return AAssetManager_fromJava(env, assets);
}

AAssetManager* contextops_get_assets() {
    if(contextops_assets_available())
        return (AAssetManager *) JNIWrapperv(&contextops_get_assets0);
    else
        return nullptr;
}
