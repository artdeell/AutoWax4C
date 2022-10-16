//
// Created by maks on 16.10.2022.
//

#ifndef USERLIB_FAKE_DLFCN_H
#define USERLIB_FAKE_DLFCN_H
extern "C" {
void *fake_dlopen(const char *libpath, int flags);
void *fake_dlsym(void *handle, const char *name);
int fake_dlclose(void *handle);
}
#endif //USERLIB_FAKE_DLFCN_H
