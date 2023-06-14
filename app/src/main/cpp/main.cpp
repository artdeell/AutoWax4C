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
#include "heartselector.h"
#include "changelevel.h"
#include "iap_purchase.h"
#include "scandecode.h"
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
static jmethodID method_authorizeKey;

static pthread_mutex_t log_mutex;
static pthread_mutex_t key_lock_mutex;
static pthread_cond_t key_lock_cond;
static bool enable_candles, enable_quests, enable_send, enable_recv, enable_fragmetns,
open_spiritshops, open_wl_collector, open_invitemanager, open_worldquests, open_changelevel,
open_heaerrtrades, open_iap_purchase;
static bool load_errored = false;
static bool changelevel_available = false;
static bool iap_purchase_available = false;
static bool key_ui_opened = false;
static int32_t accountserverclient_rel = INT32_MAX;
static _Atomic bool userWantsReauthorization = false;
static _Atomic bool userInterfaceShown = true;
static _Atomic bool edemShown = true;
static _Atomic float progressBarVal = -1;
static vm_string log_strings[6];
static char key_string[40] = {0};
static char* crash_string = "The mod did not load!";


extern char _binary_classes_dex_start[];
extern char _binary_classes_dex_size[];
extern char _binary_classes_dex_end[];

extern "C"
__attribute__ ((visibility ("default")))
func Start(){
    Init();
    return &Menu;
}

void* get_gameptr() {
    uintptr_t address =  Cipher::CipherScan("\xA8\x83\x00\xD0\x00\x10\x80\x52\x13\x39\x06\xF9\xA1\xD4\xEC\x97\x00\xE4\x00\x6F", "??x?xxxxx??x???xxxxx");
    if(address == 0) {
        __android_log_print(ANDROID_LOG_FATAL,"tinywax","pattern 0 failed");
        crash_string = locale_strings[M_AW4C_NEEDS_UPDATE];
        load_errored = true;
        return nullptr;
    }
    uintptr_t buf = 0;
    memcpy(&buf, (void *)address, 4);
    long addr;
    CipherArm64::decode_adr_imm(buf, &addr);
    int32_t rel;
    memcpy(&buf, (void *)(address + 8), 4);
    CipherArm64::decode_ldrstr_uimm(buf, &rel);
    return *(void **)(((address >> 12) << 12) + addr + rel);
}

void get_user_id(void* gameptr, char* TgcUUID) {
    uintptr_t offset1 = Cipher::CipherScan("\x68\x00\x40\xF9\xE1\x03\x1F\x2A\x00\x15\x40\xF9\x00\x00\x00\x00\xE8\x00\x40\xF9", "x?xxxxxxxxxx????x?xx");
    uintptr_t offset2 = Cipher::CipherScan("\x68\x72\x06\x91\x00\x01\x80\x3D\xFD\x7B\x41\xA9\xF4\x4F\xC2\xA8\xC0\x03\x5F\xD6", "x?xxxxxxxxxxxxxxxxxx");
    int32_t rel, rel2;
    CipherArm64::decode_ldrstr_uimm(*((uint32_t*)offset1), &rel);
    CipherArm64::decode_ldrstr_uimm(*((uint32_t*)(offset1 + 8)), &rel2);
    uintptr_t netgameclient = *(uintptr_t *)(*(uintptr_t *)((uintptr_t)gameptr + rel) + rel2);
    int32_t rel3 = CipherArm64::decode_addsub_imm(*(uint32_t*)offset2);
    auto *user = (unsigned char *)(netgameclient + rel3);
    snprintf(TgcUUID, 0x40u, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", user[0], user[1], user[2], user[3], user[4], user[5], user[6], user[7], user[8], user[9], user[10], user[11], user[12], user[13], user[14], user[15]);
}

void get_Auth(void* gameptr, char *AccountSessionToken) {
    auto* Game = (uintptr_t*)gameptr;
    if(Game == nullptr) {
        AccountSessionToken[0] = 0;
        return;
    }
    uintptr_t AccountServerClient = Game[accountserverclient_rel/8];
    auto *session = (std::byte *)(AccountServerClient + 718);
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

void* JNIWrapperv(jniexecv_t execm) {
    JNIEnv *env;bool detach = false;
    if(vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
        detach = true;
    }
    void* _return = execm(env);
    if(detach) {
        vm->DetachCurrentThread();
    }
    return _return;
}

void reloadSession(JNIEnv *env) {
    userWantsReauthorization = false;
    env->CallStaticVoidMethod(main_class, method_reauthorized);
}
void candleRun(JNIEnv* env) {
    userInterfaceShown = false;
    env->CallStaticVoidMethod(main_class, method_candleRun, enable_quests, enable_candles, enable_send, enable_recv, enable_fragmetns);
}
void edemRun(JNIEnv* env) {
    edemShown = false;
    env->CallStaticVoidMethod(main_class, method_edemRun);
}
void sumbitKey(JNIEnv *env) {
    env->CallStaticVoidMethod(main_class, method_authorizeKey, env->NewStringUTF(key_string));
}

void printLogLines() {
    pthread_mutex_lock(&log_mutex);
    for(int i = 4; i >= 0; i--) {
        if(log_strings[i].real_string != nullptr) ImGui::TextWrapped("%s", log_strings[i].real_string);
    }
    pthread_mutex_unlock(&log_mutex);
}
void Menu() {
    if(key_ui_opened) {
        ImGui::Text("Please enter key");
        ImGui::PushItemWidth(-1);
        ImGui::InputText("###key_input", key_string, 40);
        ImGui::PopItemWidth();
        if(ImGui::Button("fuck off")) {
            ThreadWrapper(&sumbitKey);
            //pthread_mutex_lock(&key_lock_mutex);
            //pthread_mutex_unlock(&key_lock_mutex);
        }
        return;
    }
    ImGui::TextUnformatted(locale_strings[OB_FREE_SOFTWARE]);
    if(!load_errored) {
        printLogLines();
        if(progressBarVal != -1)ImGui::ProgressBar(progressBarVal);
        if (userWantsReauthorization) {
            ImGui::TextUnformatted(
                    locale_strings[M_SESSION_TERMINATED]);
            if (ImGui::Button(locale_strings[M_SESSION_RELOAD])) JNIWrapper(&reloadSession);
        }
        if(userInterfaceShown) {
            ImGui::Checkbox(locale_strings[M_FRAGMENTS], &enable_fragmetns);
            ImGui::Checkbox(locale_strings[M_RUN_CANDLES], &enable_candles);
            ImGui::Checkbox(locale_strings[M_RUN_QUESTS], &enable_quests);
            ImGui::Checkbox(locale_strings[M_COLLECT_GIFTS], &enable_recv);
            ImGui::Checkbox(locale_strings[M_SEND_GIFTS], &enable_send);
            if (ImGui::Button(locale_strings[G_RUN])) JNIWrapper(&candleRun);
            if(contextops_available()) {
                ImGui::Checkbox(locale_strings[M_HEART_SELECTOR], &open_heaerrtrades);
                if(open_heaerrtrades) heartselector_draw();
            }
        }
        ImGui::Checkbox(locale_strings[M_SPIRIT_SHOPS], &open_spiritshops);
        ImGui::Checkbox(locale_strings[M_COLLECT_WL], &open_wl_collector);
        ImGui::Checkbox(locale_strings[M_INVITE_MANAGER], &open_invitemanager);
        ImGui::Checkbox(locale_strings[M_SPIRITS], &open_worldquests);
        if(changelevel_available) {
            ImGui::Checkbox(locale_strings[M_CHANGELEVEL], &open_changelevel);
            if(open_changelevel) changelevel_draw();
        }
        //if(iap_purchase_available) {
        //    ImGui::Checkbox(locale_strings[M_IAP_PURCHASE], &open_iap_purchase);
        //    if(open_iap_purchase) iap_purchase_draw();
        //}
        drop_draw();
        if(edemShown) if(ImGui::Button(locale_strings[M_EDEM_RUN]))  JNIWrapper(&edemRun);
        if(open_spiritshops) spiritshop_draw();
        if(open_wl_collector) lights_draw();
        if(open_invitemanager) invitemanager_draw();
        if(open_worldquests) worldquests_draw();
    }else{
        ImGui::TextUnformatted(crash_string);
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
void Init(){
    if(strlen(locale_strings[OB_FREE_SOFTWARE]) != 67) {
        abort();
    }
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
    method_candleRun = env->GetStaticMethodID(main_class, "candleRun", "(ZZZZZ)V");
    method_edemRun = env->GetStaticMethodID(mainClass, "edemRun", "()V");
    method_authorizeKey = env->GetStaticMethodID(mainClass, "authorizeKey", "(Ljava/lang/String;)V");
    translation_init(env);
    spiritshop_initIDs(env);
    lights_initIDs(env);
    contextops_initIDs(env);
    worldquests_initIDs(env);
    invitemanager_initIDs(env);
    heartselector_initIDs(env);
    changelevel_available = changelevel_initIDs(env);
    //iap_purchase_available = Cipher::isGameBeta();
    //if(iap_purchase_available) {
    //    iap_purchase_initIDs(env);
    //}
    scandecode_init();
    if(pthread_mutex_init(&log_mutex, nullptr) ||
       pthread_mutex_init(&key_lock_mutex, nullptr) ||
       pthread_cond_init(&key_lock_cond, nullptr)) {
        DIE("Failed to create the mutexes/conds");
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
    while(accountserverclient_rel == INT32_MAX) {
        key_ui_opened = true;
        pthread_mutex_lock(&key_lock_mutex);
        pthread_cond_wait(&key_lock_cond, &key_lock_mutex);
        pthread_mutex_unlock(&key_lock_mutex);
    }
     char user[64];
     char session[64];
     void* gameptr = get_gameptr();
     get_user_id(gameptr, user);
     get_Auth(gameptr, session);
     if(user[0] == 0 && session[0] == 0) return nullptr;
     jobjectArray credsArray = env->NewObjectArray(2, class_String, nullptr);
     env->SetObjectArrayElement(credsArray, 0, env->NewStringUTF(user));
     env->SetObjectArrayElement(credsArray, 1, env->NewStringUTF(session));
     return credsArray;
    return nullptr;
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

extern "C"
jstring
Java_git_artdeell_aw4c_CanvasMain_getUserId(JNIEnv *env, jclass clazz) {
    char uid[64];
    get_user_id(get_gameptr(), uid);
    return env->NewStringUTF(uid);
}

extern "C"
void
Java_git_artdeell_aw4c_CanvasMain_sendKeyData(JNIEnv *env, jclass clazz, jbyteArray data) {
    if(data != nullptr) {
        jbyte* dataNative = env->GetByteArrayElements(data, nullptr);
        jint dataLen = env->GetArrayLength((jarray)data);
        uintptr_t address = scandecode_run(dataNative, dataLen);
        env->ReleaseByteArrayElements(data, dataNative, JNI_ABORT);
        if(address == 0) {
            crash_string = locale_strings[M_AW4C_NEEDS_UPDATE];
            load_errored = true;
            key_ui_opened = false;
            return;
        }
        CipherArm64::decode_ldrstr_uimm(*(uint32_t*)address, &accountserverclient_rel);
        key_ui_opened = false;
    }
    pthread_cond_broadcast(&key_lock_cond); // on finish
}

const JNINativeMethod methods[] = {
        { "submitLogString",     "(Ljava/lang/String;)V", (void*)&Java_git_artdeell_aw4c_CanvasMain_submitLogString},
        {"getCredentials", "()[Ljava/lang/String;", (void*)&Java_git_artdeell_aw4c_CanvasMain_getCredentials},
        {"goReauthorize","()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_goReauthorize},
        {"submitProgressBar","(II)V", (void*)&Java_git_artdeell_aw4c_CanvasMain_submitProgressBar},
        {"unlockUI","()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_unlockUI},
        {"unlockEdem","()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_unlockEdem},
        {"getUserId", "()Ljava/lang/String;", (void*)&Java_git_artdeell_aw4c_CanvasMain_getUserId},
        {"sendKeyData", "([B)V", (void*)&Java_git_artdeell_aw4c_CanvasMain_sendKeyData}
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