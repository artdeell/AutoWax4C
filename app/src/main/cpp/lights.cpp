//
// Created by maks on 14.10.2022.
//
#include <android/log.h>
#include <unistd.h>
#include "includes/fileselector.h"
#include "includes/imgui/imgui.h"
#include "lights.h"
#include "main.h"
#include "translation.h"
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

void load_lights(int fd) {
    if(fd != -1) {
        memset(wl_input, 0, 16384);
        read(fd, wl_input, 16384);
        close(fd);
    }
}

void lights_draw() {

    ImGui::Begin(locale_strings[M_COLLECT_WL]);
    if(!ids_ok) {
        ImGui::TextUnformatted("WL init failed");
        ImGui::End();
        return;
    }
    if(runlock) {
        ImGui::InputText("###wlstring", wl_input, 16384, ImGuiInputTextFlags_CallbackAlways, &wli_cursor_pos);
        ImGui::Checkbox(locale_strings[L_WORLD_LIGHTS], &world_lights);
        ImGui::Checkbox(locale_strings[L_SPIRIT_LIGHTS], &spirit_lights);
        if (ImGui::Button(locale_strings[G_RUN])) {
            runlock = false;
            JNIWrapper(&lights_invoke);
        }
        ImGui::SameLine();
        if (ImGui::Button(locale_strings[G_CLEAR])) {
            wl_input[0] = 0;
        }
        if(contextops_available()) {
            if (ImGui::Button(locale_strings[G_COPY])) {
                contextops_setClipboard(wl_input);
            }
            ImGui::SameLine();
            if(ImGui::Button(locale_strings[G_PASTE])) {
                contextops_getClipboard(wl_input, (size_t)cursorPos, 16384);
            }
        }
        if(ImGui::Button(locale_strings[L_LOAD_FROM_FILE])) {
            requestFile("text/plain", load_lights, false);
        }
    } else {
        ImGui::TextUnformatted(locale_strings[L_COLLECTOR_RUNNING]);
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
        if(ImGui::Button(locale_strings[L_DROP])) {
            runlock = false;
            JNIWrapper(&ligts_drop);
        }
    }else{
        ImGui::TextUnformatted(locale_strings[L_COLLECTOR_RUNNING_SMALL]);
    }
}