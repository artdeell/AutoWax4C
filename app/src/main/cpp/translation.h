//
// Created by maks on 19.10.2022.
//
#include <jni.h>
#ifndef USERLIB_TRANSLATION_H
#define USERLIB_TRANSLATION_H
#define M_SESSION_TERMINATED 0
#define M_SESSION_RELOAD 1
#define M_RUN_CANDLES 2
#define M_RUN_QUESTS 3
#define M_COLLECT_GIFTS 4
#define M_SEND_GIFTS 5
#define G_RUN 6
#define M_SPIRIT_SHOPS 7
#define M_COLLECT_WL 8
#define M_INVITE_MANAGER 9
#define M_EDEM_RUN 10
#define SS_DOWNLOADING 11
#define G_LOADING 12
#define SS_LOAD_FAILED 13
#define G_RETRY 14
#define SS_OPEN 15
#define SS_BUY 16
#define G_COPY 17
#define G_PASTE 18
#define G_CLEAR 19
#define L_WORLD_LIGHTS 20
#define L_SPIRIT_LIGHTS 21
#define L_COLLECTOR_RUNNING 22
#define L_COLLECTOR_RUNNING_SMALL 23
#define L_DROP 24
#define IM_REMOVE 25
#define IM_ADD 26
#define G_BACK 27
#define IM_RELOAD 28
#define WQ_LOAD_FAILED 29
#define WQ_RUN 30
#define M_SPIRITS 31
#define OB_FREE_SOFTWARE 32
#define M_AW4C_NEEDS_UPDATE 33
#define M_FRAGMENTS 34
#define M_HEART_SELECTOR 35
#define H_FAILED_TO_LOAD_FRIENDS 36
#define H_ADD 37
#define H_REMOVE 38
#define H_SOCIOPATH 39
#define H_REFRESH 40
#define L_LOAD_FROM_FILE 41
#define M_CHANGELEVEL 42
#define J_JUMP 43
#define I_LOAD_FAILED 44
#define I_PLATFORM 45
#define I_ITEM_ID 46
#define I_BUY 47
#define M_IAP_PURCHASE 48
#define K_ENTER_KEY 49
#define K_DISCORD_URL 50
#define K_COPY_DISCORD_URL 51
#define K_SUBMIT_KEY 52
#define K_PASTE_KEY 53
#define K_SERVER_ERROR 54
#define K_NOT_FOUND 55
#define K_OUT_OF_REGISTRATIONS 56
#define K_IOE 57
#define K_UNKNOWN_ERROR 58
extern char** locale_strings;
void translation_init(JNIEnv*);
#endif //USERLIB_TRANSLATION_H
