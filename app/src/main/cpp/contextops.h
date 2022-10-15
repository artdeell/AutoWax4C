//
// Created by maks on 15.10.2022.
//

#ifndef USERLIB_CONTEXTOPS_H
#define USERLIB_CONTEXTOPS_H

void contextops_initIDs(JNIEnv*);
void contextops_getClipboard(char* buf, size_t off, size_t len);
void contextops_setClipboard(char* buf);
bool contextops_available();

#endif //USERLIB_CONTEXTOPS_H
