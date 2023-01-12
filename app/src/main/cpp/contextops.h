//
// Created by maks on 15.10.2022.
//

#ifndef USERLIB_CONTEXTOPS_H
#define USERLIB_CONTEXTOPS_H
#include <android/asset_manager.h>

void contextops_initIDs(JNIEnv*);
void contextops_getClipboard(char* buf, size_t off, size_t len);
void contextops_setClipboard(char* buf);
bool contextops_available();
bool contextops_assets_available();
AAssetManager* contextops_get_assets();

#endif //USERLIB_CONTEXTOPS_H
