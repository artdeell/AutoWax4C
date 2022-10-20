//
// Created by Lukas on 22/07/2022.
//

#include "main.h"
#include "spiritshop.h"
#include "lights.h"
#include "contextops.h"
#include "invitemanager.h"
#include "translation.h"
#include "worldquests.h"
#include "dlfake/fake_dlfcn.h"
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
static bool enable_candles, enable_quests, enable_send, enable_recv, open_spiritshops, open_wl_collector, open_invitemanager, open_worldquests;
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
__attribute__ ((visibility ("default")))
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
    userWantsReauthorization = false;
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
                    locale_strings[M_SESSION_TERMINATED]);
            if (ImGui::Button(locale_strings[M_SESSION_RELOAD])) JNIWrapper(&reloadSession);
        }
        if(userInterfaceShown) {
            ImGui::Checkbox(locale_strings[M_RUN_CANDLES], &enable_candles);
            ImGui::Checkbox(locale_strings[M_RUN_QUESTS], &enable_quests);
            ImGui::Checkbox(locale_strings[M_COLLECT_GIFTS], &enable_recv);
            ImGui::Checkbox(locale_strings[M_SEND_GIFTS], &enable_send);
            if (ImGui::Button(locale_strings[G_RUN])) JNIWrapper(&candleRun);
        }
        ImGui::Checkbox(locale_strings[M_SPIRIT_SHOPS], &open_spiritshops);
        ImGui::Checkbox(locale_strings[M_COLLECT_WL], &open_wl_collector);
        ImGui::Checkbox(locale_strings[M_INVITE_MANAGER], &open_invitemanager);
        ImGui::Checkbox(locale_strings[M_SPIRITS], &open_worldquests);
        drop_draw();
        if(edemShown) if(ImGui::Button(locale_strings[M_EDEM_RUN]))  JNIWrapper(&edemRun);
        if(open_spiritshops) spiritshop_draw();
        if(open_wl_collector) lights_draw();
        if(open_invitemanager) invitemanager_draw();
        if(open_worldquests) worldquests_draw();
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
jni_getvms_t getVMFunction_fake() {
    void* library = fake_dlopen("libart.so", 0);
    if(library == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "VMFinder","Google trolling failed. Giving up.");
        return nullptr;
    }
    void* sym = fake_dlsym(library, "JNI_GetCreatedJavaVMs");
    fake_dlclose(library);
    return (jni_getvms_t)sym;
}
jni_getvms_t getVMFunction() {
    void* library = dlopen("libnativehelper.so", RTLD_LAZY);
    if(library == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "VMFinder","Time to troll Google!");
        return getVMFunction_fake();
    }
    void* sym = dlsym(library, "JNI_GetCreatedJavaVMs");
    dlclose(library);
    return sym == nullptr ? getVMFunction_fake() : (jni_getvms_t) sym;
}
void (*original_ssl)(int64_t arg1, int64_t arg2, int64_t arg3);
void hook_ssl(int64_t arg1, int64_t arg2, int64_t arg3) {
    original_ssl(arg1, 0, 0);
}
void ssl() {
    CipherBase *hook = (new CipherHook)->set_Callback((uintptr_t) &original_ssl)
            ->set_Hook((uintptr_t) &hook_ssl)->set_Address(0xca0454, true);
    hook->Fire();
}
void Init(){
    //ssl();
    memset(log_strings, 0, sizeof(vm_string)*5);
    jsize cnt;
    JNI_GetCreatedJavaVMs_p = getVMFunction();
    if(!JNI_GetCreatedJavaVMs_p || JNI_GetCreatedJavaVMs_p(&vm, 1, &cnt) != JNI_OK || cnt == 0) {
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
    jclass class_Class = env->FindClass("java/lang/Class");
    if(!class_Class || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        DIE("Can't find the Class class");
    }
    jmethodID method_getClassLoader = env->GetMethodID(class_Class, "getClassLoader", "()Ljava/lang/ClassLoader;");

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
                                        env->CallObjectMethod(class_Class, method_getClassLoader));
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
    translation_init(env);
    spiritshop_initIDs(env);
    lights_initIDs(env);
    contextops_initIDs(env);
    worldquests_initIDs(env);
    invitemanager_initIDs(env);
    if(pthread_mutex_init(&log_mutex, nullptr)) {
        DIE("Failed to create the log mutex");
    }
    env->CallStaticVoidMethod(main_class, env->GetStaticMethodID(main_class, "init","(IZ)V"), Cipher::getGameVersion(), Cipher::isGameBeta());
}

void* threadWrapper0(void* arg) {
    JNIWrapper((jniexec_t)arg);
    return nullptr;
}
void ThreadWrapper(jniexec_t exec) {
    pthread_t thread;
    pthread_create(&thread, nullptr,&threadWrapper0, (void*)exec);
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
void FreeStringArray(char** list, jsize list_size) {
    if(list != nullptr) {
        for(jsize i = 0; i < list_size; i++) {
            if(list[i] != nullptr) free(list[i]);
        }
        free(list);
    }
}
void WriteStringOrNull(JNIEnv* env, char** string, jstring jstring) {
    if(jstring == nullptr) *string = nullptr;
    const char *idChars = env->GetStringUTFChars(jstring, nullptr);
    if (asprintf(string, "%s", idChars) == -1) {
        *string = nullptr;
    }
    env->ReleaseStringUTFChars(jstring, idChars);
    env->DeleteLocalRef(jstring);
}

