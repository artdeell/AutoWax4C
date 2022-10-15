//
// Created by maks on 14.10.2022.
//
#include <android/log.h>
#include "includes/imgui/imgui.h"
#include "lights.h"
#include "main.h"
#include "contextops.h"
static bool ids_ok = false;
static _Atomic bool runlock = true;
static jmethodID method_collectLights;
static jmethodID method_dropLights;
static bool world_lights;
static bool spirit_lights;
static char wl_input[16384] = {0};
static int dropcount;

extern "C" void
Java_git_artdeell_aw4c_CanvasMain_unlockWLCollector([[maybe_unused]] JNIEnv *env, [[maybe_unused]]jclass clazz) {
    // TODO: implement unlockWLCollector()
    runlock = true;
}
const JNINativeMethod methods[] = {
        { "unlockWLCollector",     "()V", (void*)&Java_git_artdeell_aw4c_CanvasMain_unlockWLCollector}
};

void lights_initIDs(JNIEnv* env) {
    method_collectLights = env->GetStaticMethodID(main_class, "collectLights", "(Ljava/lang/String;ZZ)V");
    method_dropLights = env->GetStaticMethodID(main_class, "dropLights", "(I)V");
    if(!method_dropLights || !method_collectLights) return;
    env->RegisterNatives(main_class, methods, sizeof(methods)/sizeof(methods[0]));
    ids_ok = true;
}
void lights_invoke(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_collectLights, env->NewStringUTF(wl_input), world_lights, spirit_lights);
}
void ligts_drop(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_dropLights, dropcount);
}
int cursorPos;
int wli_cursor_pos(ImGuiInputTextCallbackData* data) {
    cursorPos = data->CursorPos;
    return 0;
}
void lights_draw() {

    ImGui::Begin("Collect WL");
    if(!ids_ok) {
        ImGui::TextUnformatted("WL init failed");
        ImGui::End();
        return;
    }
    if(runlock) {
        ImGui::InputText("###wlstring", wl_input, 16384, ImGuiInputTextFlags_CallbackAlways, &wli_cursor_pos);
        ImGui::Checkbox("World lights", &world_lights);
        ImGui::Checkbox("Spirit lights", &spirit_lights);
        if (ImGui::Button("Run")) {
            runlock = false;
            JNIWrapper(&lights_invoke);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            wl_input[0] = 0;
        }
        if(contextops_available()) {
            if (ImGui::Button("Copy")) {
                contextops_setClipboard(wl_input);
            }
            ImGui::SameLine();
            if(ImGui::Button("Paste")) {
                contextops_getClipboard(wl_input, (size_t)cursorPos, 16384);
            }
        }
    } else {
        ImGui::TextUnformatted("The collector is running! Check log for more info");
    }
    ImGui::End();
}

void drop_draw() {
    if(!ids_ok) {
        ImGui::TextUnformatted("WL init failed");
        return;
    }
    if(runlock) {
        ImGui::InputInt("##count", &dropcount);
        if(ImGui::Button("Drop")) {
            runlock = false;
            JNIWrapper(&ligts_drop);
        }
    }else{
        ImGui::TextUnformatted("WL collector running");
    }
}