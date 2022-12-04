//
// Created by maks on 03.12.2022.
//

#include <cstdlib>
#include <android/log.h>
#include "heartselector.h"
#include "main.h"
#include "translation.h"
#include "includes/imgui/imgui.h"

static char** friendList = nullptr;
static jboolean* enableList = nullptr;
static jsize length;
static char state;
static char op;
static int selection_idx;
static char* longer_word;
static jmethodID method_heartSelector;

void hs_cmd(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_heartSelector, op, selection_idx);
}

void heartselector_draw0() {
    op = 0;
    state = 1;
    ThreadWrapper(&hs_cmd);
}
void heartselector_draw1() {
    ImGui::TextUnformatted(locale_strings[G_LOADING]);
}

float heartselector_compute_button_column_size() {
    return ImGui::CalcTextSize(longer_word).x + ImGui::GetStyle().FramePadding.x * 4;
}

void heartselector_draw2() {
    if(ImGui::Button(locale_strings[H_REFRESH])) {
        state = 0;
        return;
    }
    if(length < 1) {
        ImGui::TextUnformatted(locale_strings[H_SOCIOPATH]);
        return;
    }
    if(ImGui::BeginTable("##heartselector", 2)) {
        ImGui::TableSetupColumn("friendnames", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableSetupColumn("buttons", ImGuiTableColumnFlags_WidthFixed, heartselector_compute_button_column_size());
        ImGui::TableNextRow();
        for(jsize i = 0; i < length; i++) {
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(friendList[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if (enableList != nullptr) {
                bool enabled = enableList[i];
                if (ImGui::Button(enabled ? locale_strings[H_REMOVE] : locale_strings[H_ADD])) {
                    op = enabled ? 2 : 1;
                    selection_idx = i;
                    ThreadWrapper(&hs_cmd);
                }
            }
            ImGui::PopID();
            ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }
}

void heartselector_draw3() {
    ImGui::TextUnformatted(locale_strings[H_FAILED_TO_LOAD_FRIENDS]);
    if(ImGui::Button(locale_strings[G_RETRY])) {
        state = 0;
    }
}
void(*heartselector_states[])() = {
  heartselector_draw0, heartselector_draw1, heartselector_draw2, heartselector_draw3
};
void heartselector_draw() {
    ImGui::Begin(locale_strings[M_HEART_SELECTOR], nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    heartselector_states[state]();
    ImGui::End();
}

extern "C" void
Java_git_artdeell_autowax_heartlist_HeartList_pushList(JNIEnv *env, jclass clazz,
                                                       jobjectArray names) {
    if(friendList != nullptr) FreeStringArray(friendList, length);
    if(names == nullptr) {
        friendList = nullptr;
        state = 3;
        return;
    }
    length = env->GetArrayLength(names);
    friendList = (char**) malloc(sizeof(void*)*length);
    for(jsize i = 0; i < length; i++) {
        WriteStringOrNull(env, &friendList[i], (jstring)env->GetObjectArrayElement(names, i));
    }
    state = 2;
    // TODO: implement pushList()
}
extern "C"
void
Java_git_artdeell_autowax_heartlist_HeartList_pushEnableList(JNIEnv *env, jclass clazz,
                                                             jbooleanArray booleans) {
    jboolean* tempEnableList = enableList;
    enableList = nullptr;
    if(tempEnableList != nullptr) free(tempEnableList);
    tempEnableList = (jboolean*)malloc(sizeof(jboolean)*length);
    jboolean* nenabled = env->GetBooleanArrayElements(booleans, nullptr);
    memcpy(tempEnableList, nenabled, sizeof(jboolean)*length);
    enableList = tempEnableList;
}

const JNINativeMethod methods[] = {
        { "pushList",     "([Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_heartlist_HeartList_pushList},
        { "pushEnableList",     "([Z)V", (void*)&Java_git_artdeell_autowax_heartlist_HeartList_pushEnableList}
};

void heartselector_initIDs(JNIEnv* env) {
    jclass class_HeartSelector = LoadClass(env, "git.artdeell.autowax.heartlist.HeartList");
    env->RegisterNatives(class_HeartSelector, methods, 2);
    method_heartSelector = env->GetStaticMethodID(main_class, "heartSelector","(BI)V");
    size_t wordlen0 = strlen(locale_strings[H_ADD]);
    size_t wordlen1 = strlen(locale_strings[H_REMOVE]);
    if(wordlen0 < wordlen1 || wordlen0 == wordlen1) longer_word = locale_strings[H_REMOVE];
    if(wordlen0 > wordlen1) longer_word = locale_strings[H_ADD];
}