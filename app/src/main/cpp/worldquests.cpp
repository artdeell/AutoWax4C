//
// Created by maks on 19.10.2022.
//

#include <cstdlib>
#include <cstring>
#include "worldquests.h"
#include "main.h"
#include "includes/imgui/imgui.h"
#include "translation.h"
#include "includes/iconloader/IconLoader.h"

static char** list = nullptr;
static char** icon_list = nullptr;
static jsize list_size;
static char state = 0;
static char fail_reason[1024];
static char command = 0;
static int idx = 0;
static jmethodID method_worldQuests;

void wq_cmd(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_worldQuests, command, idx);
}

void worldquest_draw_state0() {
    state = 1;
    command = 0;
    ThreadWrapper(&wq_cmd);
}
void worldquest_draw_state1() {
    ImGui::TextUnformatted(locale_strings[G_LOADING]);
}
void worldquest_draw_state2() {
    ImGui::Text(locale_strings[WQ_LOAD_FAILED], fail_reason);
    if(ImGui::Button(locale_strings[G_RETRY])) {
        state = 0;
    }
}
float worldquest_compute_button_column_size() {
    return ImGui::CalcTextSize(locale_strings[WQ_RUN]).x + ImGui::GetStyle().FramePadding.x * 4;
}
void warldquest_draw_state3() {
    if(ImGui::BeginTable("###worldquests", 2)) {
        ImGui::TableSetupColumn("quests", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableSetupColumn("buttons", ImGuiTableColumnFlags_WidthFixed, worldquest_compute_button_column_size());
        ImGui::TableNextRow();
        for(jsize i = 0; i < list_size; i++) {
            ImGui::TableSetColumnIndex(0);
            if(icon_list[i][0] != 0) {
                if(strstr(icon_list[i], "Pendant")) {
                    SkyImage &image = IconLoader::getImage(icon_list[i]);
                    ImGui::Image(image.textureId, ImVec2(64, 64));
                }else IconLoader::icon(icon_list[i], 64);
                ImGui::SameLine();
            }
            ImGui::TextUnformatted(list[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if(ImGui::Button(locale_strings[WQ_RUN])) {
                state = 1;
                command = 1;
                idx = i;
                ThreadWrapper(&wq_cmd);
            }
            ImGui::PopID();
            ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }
}
void(*worldquest_states[])() = {
    worldquest_draw_state0, worldquest_draw_state1, worldquest_draw_state2, warldquest_draw_state3
};
void worldquests_draw() {
    ImGui::Begin(locale_strings[M_SPIRITS], nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    worldquest_states[state]();
    ImGui::End();
}

extern "C" void
Java_git_artdeell_autowax_worldquest_Spirits_onLoadResult(JNIEnv *env, [[maybe_unused]]jclass clazz,
                                                          jstring str) {
    const char* error_chars = env->GetStringUTFChars(str, nullptr);
    strncpy(fail_reason, error_chars, 1023);
    env->ReleaseStringUTFChars(str, error_chars);
    state = 2;
}
extern "C" void
Java_git_artdeell_autowax_worldquest_Spirits_onNewList(JNIEnv *env, [[maybe_unused]]jclass clazz,
                                                       jobjectArray ids, jobjectArray icons) {
    if(ids == nullptr || icons == nullptr) return;
    FreeStringArray(list, list_size);
    FreeStringArray(icon_list, list_size);
    list_size = env->GetArrayLength(ids);
    list = (char**)malloc(sizeof(void *) * list_size);
    icon_list = (char**)malloc(sizeof(void *) * list_size);
    if(!list || !icon_list) abort();
    for(jsize i = 0; i < list_size; i++) {
        WriteStringOrNull(env, &list[i], (jstring)env->GetObjectArrayElement(ids, i));
        WriteStringOrNull(env, &icon_list[i], (jstring)env->GetObjectArrayElement(icons, i));
    }
    state = 3;
}

const JNINativeMethod methods[] = {
        { "onNewList",     "([Ljava/lang/String;[Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_worldquest_Spirits_onNewList},
        { "onLoadResult",     "(Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_worldquest_Spirits_onLoadResult}
};
void worldquests_initIDs(JNIEnv* env) {
    jclass class_WorldQuests = LoadClass(env, "git.artdeell.autowax.worldquest.Spirits");
    env->RegisterNatives(class_WorldQuests, methods, 2);
    method_worldQuests = env->GetStaticMethodID(main_class, "worldQuests", "(BI)V");
}