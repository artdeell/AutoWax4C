//
// Created by maks on 14.10.2022.
//
#include <cstdlib>
#include <cstdio>
#include <__threading_support>
#include <android/log.h>
#include "spiritshop.h"
#include "translation.h"
#include "includes/imgui/imgui.h"

static bool ids_ok = false;
static bool list_done = false;

static jsize list_size = 0;
static char** list = nullptr;
static char** sub_list = nullptr;
static jlong* goto_list = nullptr;
static char net_state = 3;
static char net_init_failreason_chars[256];
static char purchase_result_chars[256];
static _Atomic char op;
static _Atomic jlong pushVal;
jmethodID method_spiritShop;
extern "C" void
Java_git_artdeell_autowax_spiritshop_SpiritShop_newList(JNIEnv *env, [[maybe_unused]] jclass clazz,
                                                        jobjectArray element_strings,
                                                        jobjectArray name_strings, jlongArray gotos) {
    FreeStringArray(list, list_size);
    FreeStringArray(sub_list, list_size);
    if(goto_list != nullptr)
        free(goto_list);
    list_size = env->GetArrayLength(element_strings);
    list = (char**)malloc(sizeof(void *) * list_size);
    sub_list = (char**)malloc(sizeof(void *) * list_size);
    goto_list = (jlong*) malloc(sizeof(jlong) * list_size);
    if(!list || !sub_list || !goto_list) abort();
    jlong* ngotos = env->GetLongArrayElements(gotos, nullptr);
    memcpy(goto_list, ngotos, sizeof(jlong)*list_size);
    env->ReleaseLongArrayElements(gotos, ngotos, JNI_ABORT);
    for(jsize i = 0; i < list_size; i++) {
        WriteStringOrNull(env, &sub_list[i], (jstring)env->GetObjectArrayElement(element_strings, i));
        WriteStringOrNull(env, &list[i], (jstring)env->GetObjectArrayElement(name_strings, i));
    }
    list_done = true;
}
extern "C" void
Java_git_artdeell_autowax_spiritshop_SpiritShop_initDone(JNIEnv *env, [[maybe_unused]] jclass clazz,
                                                         jstring result) {
    if(result == nullptr) {
        net_state = 2;
    }else{
        const char* scs = env->GetStringUTFChars(result, nullptr);
        snprintf(purchase_result_chars, 255, "%s", net_init_failreason_chars);
        env->ReleaseStringUTFChars(result, scs);
        net_state = 1;
    }
}
extern "C" void
Java_git_artdeell_autowax_spiritshop_SpiritShop_purchaseResult(JNIEnv *env, [[maybe_unused]] jclass clazz,
                                                               jstring result) {
    const char* result_chars = env->GetStringUTFChars(result, nullptr);
    snprintf(purchase_result_chars, 255, "%s", result_chars);
    env->ReleaseStringUTFChars(result, result_chars);
}
const JNINativeMethod methods[] = {
        { "newList",     "([Ljava/lang/String;[Ljava/lang/String;[J)V", (void*)&Java_git_artdeell_autowax_spiritshop_SpiritShop_newList},
        { "initDone",     "(Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_spiritshop_SpiritShop_initDone},
        {"purchaseResult", "(Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_spiritshop_SpiritShop_purchaseResult}
};
void spiritshop_initIDs(JNIEnv* env) {
    jclass class_SpiritShop = LoadClass(env, "git.artdeell.autowax.spiritshop.SpiritShop");
    if(class_SpiritShop == nullptr) return;
    env->RegisterNatives(class_SpiritShop, methods, sizeof(methods)/sizeof(methods[0]));
    method_spiritShop = env->GetStaticMethodID(main_class, "spiritShop", "(BJ)V");
    if(method_spiritShop == nullptr) return;
    ids_ok = true;
}
void doMagikJNI(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_spiritShop,(char) op, (jlong) pushVal);
}


void ss_draw_net0() {
    ImGui::TextUnformatted(locale_strings[SS_DOWNLOADING]);
}
void ss_draw_net4() {
    net_state = op = 0;
    ThreadWrapper(&doMagikJNI);
}
void ss_draw_net1() {
    ImGui::Text(locale_strings[SS_LOAD_FAILED], net_init_failreason_chars);
    if(ImGui::Button(locale_strings[G_RETRY])) ss_draw_net4();
}
float shop_compute_button_column_size() {
    float size = ImGui::CalcTextSize(locale_strings[SS_OPEN]).x + ImGui::GetStyle().FramePadding.x * 4;
        size += ImGui::CalcTextSize(locale_strings[SS_BUY]).x + ImGui::GetStyle().FramePadding.x * 4;
    return size;
}
void ss_draw_net2() {
    if (list_done) {
        if(ImGui::Button(locale_strings[G_BACK])) {
            list_done = false;
            op = 2;
            ThreadWrapper(&doMagikJNI);
        }
        if(purchase_result_chars[0] != 0) {
            ImGui::SameLine();
            ImGui::Text("%s", purchase_result_chars);
            ImGui::SameLine();
            if(ImGui::Button("OK")) purchase_result_chars[0] = 0;
        }
        if (ImGui::BeginTable("#spirit_shop", 2)) {
            float width = shop_compute_button_column_size();
            ImGui::TableSetupColumn("names", ImGuiTableColumnFlags_WidthStretch, 1);
            ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed, width);
            ImGui::TableNextRow();
            for (jsize i = 0; i < list_size; i++) {
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", list[i]);
                ImGui::Text("%s", sub_list[i]);
                ImGui::TableSetColumnIndex(1);
                ImGui::PushID(list[i]);
                if (ImGui::Button(locale_strings[SS_OPEN])) {
                    list_done = false;
                    op = 1;
                    pushVal = goto_list[i];
                    ThreadWrapper(&doMagikJNI);
                }
                ImGui::SameLine();
                if (ImGui::Button(locale_strings[SS_BUY])) {
                    purchase_result_chars[0] = 0;
                    op = 3;
                    pushVal = goto_list[i];
                    ThreadWrapper(&doMagikJNI);
                }
                ImGui::PopID();
                ImGui::TableNextRow();
            }
            ImGui::EndTable();
        }
    } else {
        ImGui::TextUnformatted(locale_strings[G_LOADING]);
    }
}
void(*ns_draw[4])() = {ss_draw_net0, ss_draw_net1, ss_draw_net2, ss_draw_net4};
void spiritshop_draw() {
    ImGui::Begin(locale_strings[M_SPIRIT_SHOPS], nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if(ids_ok) {
        ns_draw[net_state]();
    }else{
        ImGui::TextUnformatted("Failed to initialize");
    }
    ImGui::End();
}


