//
// Created by Lukas on 01/08/2022.
//

#ifndef USERLIB_MAIN_H
#define USERLIB_MAIN_H

#include <jni.h>

typedef void(*jniexec_t)(JNIEnv*);
void ThreadWrapper(jniexec_t exec);
void JNIWrapper(jniexec_t);
jclass LoadClass(JNIEnv*,const char*);
extern jclass main_class;

typedef void (*func)();
void Menu();
void Init();




#endif //USERLIB_MAIN_H
