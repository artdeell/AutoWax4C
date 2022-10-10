//
// Created by Lukas on 22/07/2022.
//

#include "main.h"
#include "includes/cipher/Cipher.h"
#include "includes/imgui/imgui.h"
#include "includes/misc/Logger.h"
#include <jni.h>
#include <dlfcn.h>

static bool load_errored;
typedef  jint(*jni_getvms_t)(JavaVM**, jsize, jsize*);
static jni_getvms_t JNI_GetCreatedJavaVMs_p;
static jclass main_class;
static JavaVM *vm;
#define DIE(x) LOGE(x); load_errored = true; return

extern char _binary_classes_dex_start[];
extern char _binary_classes_dex_size[];
extern char _binary_classes_dex_end[];

void Menu() {
}


void Init(){
        jsize cnt;
        JNI_GetCreatedJavaVMs_p = (jni_getvms_t) dlsym(dlopen("libnativehelper.so", RTLD_LAZY), "JNI_GetCreatedJavaVMs");
        if(JNI_GetCreatedJavaVMs_p(&vm, 1, &cnt) != JNI_OK || cnt == 0) {
            DIE("Failed to find a JVM");
        }
        JNIEnv* env;
        jint result = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (result == JNI_EDETACHED) {
            result = vm->AttachCurrentThread(&env, nullptr);
        }
        if (result != JNI_OK) {
            DIE("Can't get a JNIEnv");
        }
        jclass class_InMemoryClassLoader = env->FindClass("dalvik/system/InMemoryDexClassLoader");
        jmethodID constructor_InMemoryClassLoader = env->GetMethodID(class_InMemoryClassLoader, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
        jmethodID method_loadClass = env->GetMethodID(class_InMemoryClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        jobject byteBuffer = env->NewDirectByteBuffer(_binary_classes_dex_start, _binary_classes_dex_end - _binary_classes_dex_start);
        LOGI("Creating the class loader...");
        jobject inMemoryClassLoader = env->NewObject(class_InMemoryClassLoader, constructor_InMemoryClassLoader, byteBuffer,
                                                     nullptr);
        jstring className = env->NewStringUTF("git.artdeell.aw4c.CanvasMain");
        jobject mainClass = env->CallObjectMethod(inMemoryClassLoader, method_loadClass, className);
        if(env->ExceptionCheck()) {
            env->ExceptionDescribe();
        }
        LOGI("ClassLoader class = %p", mainClass);
        main_class = (jclass) env->NewGlobalRef(mainClass);
        jmethodID method_intFromJava = env->GetStaticMethodID(main_class, "intFromJava", "()I");
        LOGI("%i", env->CallStaticIntMethod(main_class, method_intFromJava));
};