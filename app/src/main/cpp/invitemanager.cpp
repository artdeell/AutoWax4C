//
// Created by maks on 15.10.2022.
//

#include <__threading_support>
#include "invitemanager.h"
#include "main.h"
#include "contextops.h"
#include "includes/imgui/imgui.h"
#include <cstdlib>
#include <cstdio>
static char status = 0;
static char op = 0;
static int val = 0;
static char** c_invites = nullptr;
static jsize invites_count = 0;
static jmethodID method_InviteManager;
static jmethodID method_CreateInvite;
static char invite_buf[1024] = {0};


extern "C"
void
Java_git_artdeell_autowax_invitemanager_InviteManager_onInviteList(JNIEnv *env, [[maybe_unused]]jclass clazz,
                                                                   jobjectArray invites) {
    if(invites == nullptr) {
        status = 0;
    }
    if(c_invites != nullptr) {
        for(jsize i = 0; i < invites_count; i++) {
            free(c_invites[i]);
        }
        free(c_invites);
    }
    invites_count = env->GetArrayLength(invites);
    c_invites = (char**)malloc(invites_count*(sizeof (void*)));
    for(jsize i = 0; i < invites_count;  i++) {
        auto invite = (jstring)env->GetObjectArrayElement(invites, i);
        const char* invite_chars = env->GetStringUTFChars(invite, nullptr);
        if(asprintf(&c_invites[i], "%s", invite_chars) == -1) {
            c_invites[i] = nullptr;
        }
        env->ReleaseStringUTFChars(invite,invite_chars);
        env->DeleteLocalRef(invite);
    }
    status = 2;
}
const JNINativeMethod methods[] = {
        { "onInviteList",     "([Ljava/lang/String;)V", (void*)&Java_git_artdeell_autowax_invitemanager_InviteManager_onInviteList}
};
void invitemanager_initIDs(JNIEnv* env) {
    method_InviteManager = env->GetStaticMethodID(main_class, "inviteManager", "(BI)V");
    method_CreateInvite = env->GetStaticMethodID(main_class, "createInvite", "(Ljava/lang/String;)V");
    env->RegisterNatives(LoadClass(env, "git.artdeell.autowax.invitemanager.InviteManager"), methods, 1);
}
void invitemanager_op(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_InviteManager, op, val);
}
void invitemanager_create(JNIEnv* env) {
    env->CallStaticVoidMethod(main_class, method_CreateInvite, env->NewStringUTF(invite_buf));
}

void ivm_status_handler0() {
    status = 1;
    op = 2;
    ThreadWrapper(&invitemanager_op);
}
void ivm_status_handler1() {
    ImGui::Text("Loading...");
}
float ivm_compute_button_column_size() {
    float size = ImGui::CalcTextSize("Remove").x + ImGui::GetStyle().FramePadding.x * 4;
    if(contextops_available()) {
        size += ImGui::CalcTextSize("Copy").x + ImGui::GetStyle().FramePadding.x * 4;
    }
    return size;
}
void ivm_status_handler2() {
    if(ImGui::Button("Reload")) {
        status = 0;
    }
    ImGui::InputText("###username", invite_buf, 1024);
    ImGui::SameLine();
    if(ImGui::Button("Add")) {
        status = 1;
        ThreadWrapper(&invitemanager_create);
    }
    if(ImGui::BeginTable("###invites", 2)) {
        float width = ivm_compute_button_column_size();
        ImGui::TableSetupColumn("usernames", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableSetupColumn("buttons", ImGuiTableColumnFlags_WidthFixed, width);
        ImGui::TableNextRow();
        for(jsize i = 0; i < invites_count; i++) {
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(c_invites[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if (contextops_available()) {
                if (ImGui::Button("Copy")) {
                    op = 0;
                    val = i;
                    ThreadWrapper(&invitemanager_op);
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Remove")) {
                status = 1;
                op = 1;
                val = i;
                ThreadWrapper(&invitemanager_op);
            }
            ImGui::PopID();
            ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }

}
void (*status_handlers[3])()= {ivm_status_handler0, ivm_status_handler1, ivm_status_handler2};
void invitemanager_draw() {
    ImGui::Begin("Invite Manager", nullptr,ImGuiWindowFlags_AlwaysAutoResize);
    status_handlers[status]();
    ImGui::End();
}

