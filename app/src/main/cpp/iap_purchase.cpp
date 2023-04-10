//
// Created by maks on 06.04.2023.
//

#include <cstdlib>
#include "iap_purchase.h"
#include "main.h"
#include "translation.h"
#include "includes/imgui/imgui.h"
#include "includes/iconloader/IconLoader.h"

static jmethodID method_iapPurchaseReload;
static jmethodID method_iapPurchaseBuy;
static unsigned char idlist_state = 0;
static char** idlist_names;
static char** idlist_ids;
static char** idlist_icons;
static jsize idlist_count;
static char platform_string[512] = "android";
static char id_string[1024] = {0};
static char purchase_result[1024] = {0};
void callReload(JNIEnv *env) {
    env->CallStaticVoidMethod(main_class, method_iapPurchaseReload, env->NewStringUTF(platform_string));
}

void callBuy(JNIEnv *env) {
    env->CallStaticVoidMethod(main_class, method_iapPurchaseBuy, env->NewStringUTF(platform_string), env->NewStringUTF(id_string));
}

void iap_purchase_draw_idlist0() {
    ThreadWrapper(&callReload);
    idlist_state = 1;
}
void iap_purchase_draw_idlist1() {
    ImGui::TextUnformatted(locale_strings[G_LOADING]);
}
void iap_purchase_draw_idlist2() {
    ImGui::TextUnformatted(locale_strings[I_PLATFORM]);
    ImGui::InputText("###platform", platform_string, 512);
    ImGui::SameLine();
    if(ImGui::Button("OK")) {
        idlist_state = 0;
        return;
    }
    //ImGui::ListBox("###iap_purchase", &listBoxItemCurrent, idlist_names, idlist_count);
    if(ImGui::BeginTable("###iap_purchase", 2, ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("names", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableSetupColumn("buttons", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(">").x + ImGui::GetStyle().FramePadding.x * 4);
        ImGui::TableNextRow();
        for(jsize i = 0; i < idlist_count; i++) {
            ImGui::TableSetColumnIndex(0);
            if(idlist_icons[i] != nullptr && !strstr(idlist_icons[i], "{{")) {
                IconLoader::icon(std::string(idlist_icons[i]), 64);
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(idlist_names[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(idlist_names[i]);
            if(ImGui::Button(">")) {
                strncpy(id_string, idlist_ids[i], 1024);
            }
            ImGui::PopID();
            ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }
}
void iap_purchase_draw_idlist3() {
    ImGui::TextUnformatted(locale_strings[I_LOAD_FAILED]);
    if(ImGui::Button(locale_strings[G_RETRY])) {
        idlist_state = 0;
    }
}

void (*idlist_states[])() = {iap_purchase_draw_idlist0, iap_purchase_draw_idlist1, iap_purchase_draw_idlist2, iap_purchase_draw_idlist3};

void iap_purchase_draw() {
    ImGui::Begin(locale_strings[M_IAP_PURCHASE]);
    idlist_states[idlist_state]();
    ImGui::TextUnformatted(locale_strings[I_ITEM_ID]);
    ImGui::PushItemWidth(-1);
    ImGui::InputText("###item_id", id_string, 1024);
    ImGui::PopItemWidth();
    if(purchase_result[0] != 0) {
        ImGui::TextUnformatted(purchase_result);
        if(ImGui::Button("OK")) purchase_result[0] = 0;
    }
    if(ImGui::Button(locale_strings[I_BUY]) && id_string[0] != 0) {
        ThreadWrapper(callBuy);
    }
    ImGui::End();
}

extern "C" void
Java_git_artdeell_autowax_iap_IapPurchase_onIapList(JNIEnv *env, jclass clazz, jobjectArray iaps, jobjectArray names, jobjectArray icons) {
    FreeStringArray(idlist_names, idlist_count);
    FreeStringArray(idlist_ids, idlist_count);
    FreeStringArray(idlist_icons, idlist_count);
    if(iaps == nullptr) {
        idlist_state = 3;
        return;
    }
    idlist_count = env->GetArrayLength(iaps);
    idlist_names = (char**)malloc(idlist_count*sizeof(void*));
    idlist_ids = (char**)malloc(idlist_count*sizeof(void*));
    idlist_icons = (char**)malloc(idlist_count*sizeof(void*));
    for(jsize i = 0; i < idlist_count; i++) {
        WriteStringOrNull(env, &idlist_names[i], (jstring)env->GetObjectArrayElement(names, i));
        WriteStringOrNull(env, &idlist_ids[i], (jstring)env->GetObjectArrayElement(iaps, i));
        WriteStringOrNull(env, &idlist_icons[i], (jstring)env->GetObjectArrayElement(icons, i));
    }
    idlist_state = 2;
}

extern "C" void
Java_git_artdeell_autowax_iap_IapPurchase_onIapResult(JNIEnv *env, jclass clazz, jstring result) {
    // TODO: implement onIapResult()
    const char* cresult = env->GetStringUTFChars(result, nullptr);
    strncpy(purchase_result, cresult, 1024);
    env->ReleaseStringUTFChars(result, cresult);
}

const JNINativeMethod methods[] = {
        {"onIapList", "([Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_iap_IapPurchase_onIapList},
        {"onIapResult", "(Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_iap_IapPurchase_onIapResult}
};

void iap_purchase_initIDs(JNIEnv* env) {
    jclass iap_purchase = LoadClass(env, "git.artdeell.autowax.iap.IapPurchase");
    env->RegisterNatives(iap_purchase, methods, 2);
    method_iapPurchaseReload = env->GetStaticMethodID(main_class, "iapReloadList", "(Ljava/lang/String;)V");
    method_iapPurchaseBuy = env->GetStaticMethodID(main_class, "iapBuy", "(Ljava/lang/String;Ljava/lang/String;)V");
}