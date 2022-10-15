//
// Created by maks on 15.10.2022.
//
#include <cstring>
#include <android/log.h>
#include "main.h"
#include "contextops.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

bool ids_ok = false;
jclass class_ContextOps;
jmethodID method_getClipboard;
jmethodID method_setClipboard;
void contextops_initIDs(JNIEnv* env) {
    class_ContextOps = LoadClass(env, "git.artdeell.aw4c.ContextOps");
    if(!class_ContextOps) return;
    class_ContextOps = (jclass) env->NewGlobalRef(class_ContextOps);
    method_getClipboard = env->GetStaticMethodID(class_ContextOps, "getClipboard", "()Ljava/lang/String;");
    method_setClipboard = env->GetStaticMethodID(class_ContextOps, "setClipboard", "(Ljava/lang/String;)V");
    if(!method_setClipboard || !method_getClipboard) return;
    ids_ok = env->CallStaticBooleanMethod(class_ContextOps, env->GetStaticMethodID(class_ContextOps, "init", "()Z"));
}
char* cbuf;
size_t coff;
size_t clen;
void contextops_getClipboard0(JNIEnv* env) {
    jstring clipstr = (jstring) env->CallStaticObjectMethod(class_ContextOps, method_getClipboard);
    if(clipstr == nullptr) return;
    const char* chars = env->GetStringUTFChars(clipstr, nullptr);
    strncpy(cbuf+coff, chars, MIN(env->GetStringUTFLength(clipstr), clen - coff));
    env->ReleaseStringUTFChars(clipstr, chars);
}
void contextops_setClipboard0(JNIEnv *env) {
    env->CallStaticVoidMethod(class_ContextOps,method_setClipboard, env->NewStringUTF(cbuf));
}
void contextops_getClipboard(char* buf, size_t off, size_t len) {
    if(ids_ok) {
        cbuf = buf;
        coff = off;
        clen = len;
        JNIWrapper(&contextops_getClipboard0);
    }
}
void contextops_setClipboard(char* buf) {
    if(ids_ok) {
        cbuf = buf;
        JNIWrapper(&contextops_setClipboard0);
    }
}
bool contextops_available() {
    return ids_ok;
}
