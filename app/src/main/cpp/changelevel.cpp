//
// Created by maks on 12.01.2023.
//

#include <cstring>
#include <cstdlib>
#include <android/log.h>
#include "changelevel.h"
#include "contextops.h"
#include "main.h"
#include "includes/imgui/imgui.h"
#include "translation.h"
#include "includes/cipher/Cipher.h"

static char** level_names;
static jsize level_count;
typedef void* (*ChangeLevelFptr)(void*, void*, const char*);
ChangeLevelFptr changelevel_func;
bool changelevel_initIDs(JNIEnv* env) {
    if(contextops_assets_available()) {
        uintptr_t changelevel_foff = Cipher::CipherScan("\x33\xF4\x42\xF9\xF4\x03\x02\xAA\x7C\xC2\xFF\x97\xE1\x03\x00\xAA\xE0\x03\x13\xAA", "x?xxxxxx???xxxxxxxxx");
        if(changelevel_foff == 0) return false;
        changelevel_func = (ChangeLevelFptr)(changelevel_foff - 0x14);
        jmethodID method_getLevelNames = env->GetStaticMethodID(main_class, "getLevelNames", "()[Ljava/lang/String;");
        auto levelNames = (jobjectArray)env->CallStaticObjectMethod(main_class, method_getLevelNames);
        level_count = env->GetArrayLength(levelNames);
        level_names = (char**)malloc(level_count * sizeof (char**));
        for(jsize i = 0; i < level_count; i++) {
            auto element = (jstring) env->GetObjectArrayElement(levelNames, i);
            WriteStringOrNull(env, &level_names[i], element);
        }
        return true;
    }
    return false;
}
void changelevel_draw() {
    ImGui::Begin(locale_strings[M_CHANGELEVEL]);
    if(ImGui::BeginTable("##changelevel", 2)) {
        ImGui::TableSetupColumn("##names", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableSetupColumn("##buttons", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(locale_strings[J_JUMP]).x + ImGui::GetStyle().FramePadding.x * 4);
        ImGui::TableNextRow();
        for(jsize i = 0; i < level_count; i++) {

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(level_names[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if(ImGui::Button(locale_strings[J_JUMP])) {
                void* game = get_gameptr();
                if(game != nullptr) {
                    changelevel_func(nullptr, game, level_names[i]);
                }
            }
            ImGui::PopID();
            ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }
    ImGui::End();
}