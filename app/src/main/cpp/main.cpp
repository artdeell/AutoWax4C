//
// Created by Lukas on 22/07/2022.
//

#include "main.h"
#include "spiritshop.h"
#include "lights.h"
#include "contextops.h"
#include "includes/cipher/Cipher.h"
#include "includes/imgui/imgui.h"
#include "includes/misc/Logger.h"
#include "includes/cipher/CipherArm64.h"
#include <dlfcn.h>

#define DIE(x) LOGE(x); load_errored = true; return

void registerNatives(JNIEnv*);

typedef struct {
    jstring jni_string;
    const char* real_string;
} vm_string;
typedef  jint(*jni_getvms_t)(JavaVM**, jsize, jsize*);

static jni_getvms_t JNI_GetCreatedJavaVMs_p;
static JavaVM *vm;

jclass main_class;
static jclass class_String;
static jobject object_classLoader;
static jmethodID method_reauthorized;
static jmethodID method_candleRun;
static jmethodID method_edemRun;
static jmethodID method_loadClass;

static pthread_mutex_t log_mutex;
static bool enable_candles, enable_quests, enable_send, enable_recv, open_spiritshops, open_wl_collector;
static bool load_errored = false;
static _Atomic bool userWantsReauthorization = false;
static _Atomic bool userInterfaceShown = true;
static _Atomic bool edemShown = true;
static _Atomic float progressBarVal = -1;
static vm_string log_strings[6];



extern char _binary_classes_dex_start[];
extern char _binary_classes_dex_size[];
extern char _binary_classes_dex_end[];

extern "C"
func Start(){
    Init();
    return &Menu;
}

void get_Auth(char *TgcUUID, char *AccountSessionToken){
    uintptr_t address =  Cipher::CipherScan("\x00\x00\x00\xF0\x00\x00\x00\x00\x13\x00\x00\xF9\x00\x00\x00\x97\x00\xE4\x00\x6F", "??xx????x??x???xxxxx");
    if(address == 0) {
        __android_log_print(ANDROID_LOG_FATAL,"tinywax","pattern 0 failed");
        abort();
    }
    uintptr_t buf = 0;
    memcpy(&buf, (void *)address, 4);
    long addr;
    CipherArm64::decode_adr_imm(buf, &addr);
    int32_t rel;
    memcpy(&buf, (void *)(address + 8), 4);
    CipherArm64::decode_ldrstr_uimm(buf, &rel);
    uintptr_t *Game = *(uintptr_t **)(((address >> 12) << 12) + addr + rel);
    address = Cipher::CipherScan("\x00\x00\x40\xF9\x00\x00\xF0\x97\x00\x00\x00\x36\x00\x00\x40\xF9\x00\x00\x41\xF9", "??xx??xx??xx??xxx?xx");
    if(address == 0) {
        __android_log_print(ANDROID_LOG_FATAL,"tinywax","pattern 1 failed");
        abort();
    }
    memcpy(&buf, (void *)address, 4);
    CipherArm64::decode_ldrstr_uimm(buf, &rel);
    uintptr_t AccountServerClient = Game[rel/8];
    auto *user = (std::byte *)(AccountServerClient + 702);
    auto *session = (std::byte *)(AccountServerClient + 718);
    snprintf(TgcUUID, 0x40u, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", user[0], user[1], user[2], user[3], user[4], user[5], user[6], user[7], user[8], user[9], user[10], user[11], user[12], user[13], user[14], user[15]);
    snprintf(AccountSessionToken, 0x40u ,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", session[0], session[1], session[2], session[3], session[4], session[5], session[6], session[7], session[8], session[9], session[10], session[11], session[12], session[13], session[14], session[15]);
}

void JNIWrapper(jniexec_t execm) {
    JNIEnv *env;bool detach = false;
    if(vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
        detach = true;
    }
    execm(env);
    if(detach) {
        vm->DetachCurrentThread();
    }
}
void reloadSession(JNIEnv *env) {
    env->CallStaticVoidMethod(main_class, method_reauthorized);
}
void candleRun(JNIEnv* env) {
    userInterfaceShown = false;
    env->CallStaticVoidMethod(main_class, method_candleRun, enable_candles, enable_quests, enable_send, enable_recv);
}
void edemRun(JNIEnv* env) {
    edemShown = false;
    env->CallStaticVoidMethod(main_class, method_edemRun);
}
void printLogLines() {
    pthread_mutex_lock(&log_mutex);
    for(int i = 4; i >= 0; i--) {
        if(log_strings[i].real_string != nullptr) ImGui::TextWrapped("%s", log_strings[i].real_string);
    }
    pthread_mutex_unlock(&log_mutex);
}
void Menu() {
    if(!load_errored) {
        printLogLines();
        if(progressBarVal != -1)ImGui::ProgressBar(progressBarVal);
        if (userWantsReauthorization) {
            ImGui::TextUnformatted(
                    "It seems like the current session was terminated. \nPress the button below when you are ready to continue.");
            if (ImGui::Button("Reload session")) JNIWrapper(&reloadSession);
        }
        if(userInterfaceShown) {
            ImGui::Checkbox("Run candles", &enable_candles);
            ImGui::Checkbox("Run quests", &enable_quests);
            ImGui::Checkbox("Collect gifts", &enable_recv);
            ImGui::Checkbox("Send gifts", &enable_send);
            if (ImGui::Button("Run")) JNIWrapper(&candleRun);
        }
        ImGui::Checkbox("Spirit Shops", &open_spiritshops);
        ImGui::Checkbox("Collect WL", &open_wl_collector);
        drop_draw();
        if(edemShown) if(ImGui::Button("Edem run"))  JNIWrapper(&edemRun);
        if(open_spiritshops) spiritshop_draw();
        if(open_wl_collector) lights_draw();
    }else{
        ImGui::Text("The mod did not load!");
    }
}
jclass LoadClass(JNIEnv* env, const char* name) {
    auto clazz = (jclass)env->CallObjectMethod(object_classLoader, method_loadClass, env->NewStringUTF(name));
    if(env->ExceptionCheck()) {
        env->ExceptionDescribe();
        LOGE("Failed to load %s", name);
        return nullptr;
    }
    return clazz;
}
void Init(){
    memset(log_strings, 0, sizeof(vm_string)*5);
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
    if(!class_InMemoryClassLoader || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        DIE("Can't find the class loader");
    }
    jmethodID constructor_InMemoryClassLoader = env->GetMethodID(class_InMemoryClassLoader, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    method_loadClass = env->GetMethodID(class_InMemoryClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if(!constructor_InMemoryClassLoader || !method_loadClass){
        DIE("Can't find the class methods");
    }
    jobject byteBuffer = env->NewDirectByteBuffer(_binary_classes_dex_start, _binary_classes_dex_end - _binary_classes_dex_start);
    if(!byteBuffer) {
        DIE("Can't create the byte buffer");
    }
    object_classLoader = env->NewObject(class_InMemoryClassLoader, constructor_InMemoryClassLoader, byteBuffer,
                                        (jobject) nullptr);
    if(env->ExceptionCheck()) {
        env->ExceptionDescribe();
        DIE("Failed to create class loader");
    }
    object_classLoader = env->NewGlobalRef(object_classLoader);
    jclass mainClass = LoadClass(env,"git.artdeell.aw4c.CanvasMain");
    if(!mainClass) {
        DIE("Failed to load main class");
    }
    LOGI("ClassLoader class = %p", mainClass);
    class_String = (jclass)env->NewGlobalRef(env->FindClass("java/lang/String"));
    main_class = (jclass) env->NewGlobalRef(mainClass);
    registerNatives(env);

    method_reauthorized = env->GetStaticMethodID(main_class, "reauthorized","()V");
    method_candleRun = env->GetStaticMethodID(main_class, "candleRun", "(ZZZZ)V");
    method_edemRun = env->GetStaticMethodID(mainClass, "edemRun", "()V");
    spiritshop_initIDs(env);
    lights_initIDs(env);
    contextops_initIDs(env);
    if(pthread_mutex_init(&log_mutex, nullptr)) {
        DIE("Failed to create the log mutex");
    }
    env->CallStaticVoidMethod(main_class, env->GetStaticMethodID(main_class, "init","(IZ)V"), Cipher::getGameVersion(), Cipher::isGameBeta());
}


extern "C"
 jobjectArray 
Java_git_artdeell_aw4c_CanvasMain_getCredentials(JNIEnv *env, [[maybe_unused]]jclass clazz) {
    char user[64];
    char session[64];
    get_Auth(user, session);
    jobjectArray credsArray = env->NewObjectArray(2, class_String, nullptr);
    env->SetObjectArrayElement(credsArray, 0, env->NewStringUTF(user));
    env->SetObjectArrayElement(credsArray, 1, env->NewStringUTF(session));
    return credsArray;
}
extern "C"
 void 
Java_git_artdeell_aw4c_CanvasMain_goReauthorize([[maybe_unused]]JNIEnv *env, [[maybe_unused]]jclass clazz) {
        userWantsReauthorization = true;
}
extern "C"
 void 
Java_git_artdeell_aw4c_CanvasMain_submitLogString(JNIEnv *env, [[maybe_unused]]jclass clazz, jstring s) {
    pthread_mutex_lock(&log_mutex);
    log_strings[5] = log_strings[4];
    log_strings[4] = log_strings[3];
    log_strings[3] = log_strings[2];
    log_strings[2] = log_strings[1];
    log_strings[1] = log_strings[0];
    log_strings[0].jni_string =  (jstring)env->NewGlobalRef((jobject)s);
    log_strings[0].real_string = env->GetStringUTFChars(s, nullptr);
    if(log_strings[5].real_string != nullptr && log_strings[5].jni_string) {
        env->ReleaseStringUTFChars(log_strings[5].jni_string, log_strings[5].real_string);
        env->DeleteGlobalRef(log_strings[5].jni_string);
    }
    pthread_mutex_unlock(&log_mutex);
}
extern "C"
 void 
Java_git_artdeell_aw4c_CanvasMain_submitProgressBar([[maybe_unused]]JNIEnv *env, [[maybe_unused]]jclass clazz, jint cur, jint max) {
        if(max == -1) progressBarVal = -1;
        else progressBarVal = (float)cur / (float)max;
}
extern "C"
void
Java_git_artdeell_aw4c_CanvasMain_unlockUI([[maybe_unused]]JNIEnv *env, [[maybe_unused]]jclass clazz) {
    userInterfaceShown = true;
}
extern "C"
void
Java_git_artdeell_aw4c_CanvasMain_unlockEdem([[maybe_unused]]JNIEnv *env, [[maybe_unused]]jclass clazz) {
    edemShown = true;
}
const JNINativeMethod methods[] = {
        { "submitLogString",     "(Ljava/lang/String;)V", (void*)&Java_git_artdeell_aw4c_CanvasMain_submitLogString},
        {"getCredentials", "()[Ljava/lang/String;", (void*)&Java_git_artdeell_aw4c_CanvasMain_getCredentials},
        {"goReauthorize","()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_goReauthorize},
        {"submitProgressBar","(II)V", (void*)&Java_git_artdeell_aw4c_CanvasMain_submitProgressBar},
        {"unlockUI","()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_unlockUI},
        {"unlockEdem","()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_unlockEdem},
};
void registerNatives(JNIEnv* env) {
    env->RegisterNatives(main_class, methods, sizeof(methods)/sizeof(methods[0]));
}

