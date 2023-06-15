//
// Created by maks on 16.05.2023.
//

#ifndef USERLIB_SCANDECODE_H
#define USERLIB_SCANDECODE_H
#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include "includes/cipher/Cipher.h"


extern uintptr_t skybase;
extern size_t skylen;

__attribute((always_inline)) inline unsigned long sdstrtoul(const char *nptr) {
    const int base = 16;
    const char *s = nptr;
    unsigned long acc;
    int c;
    unsigned long cutoff;
    int any, cutlim;
    do {
        c = *s++;
    } while (c == ' ' || c == '\t');
    if (c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
    }
    cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (c >= 48 & c <= 57)
            c -= '0';
        else if (c >= 65 & c <= 70)
            c -= 'A' - 10;
        else if(c >= 97 & c <= 102)
            c -= 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULONG_MAX;
    }
    return (acc);
}

__attribute((always_inline)) void inline scandecode_line_process(char* line, size_t count) {
    const char* search_target = "libBootloader.so";
    const size_t target_len = strlen(search_target);
    char* end = line + count - target_len;
    size_t i = 0;
    while(true) {
        if(end[i] == 0) break;
        else if(end[i] != search_target[i]) return;
        i++;
    }
    uintptr_t memstart, memend;
    size_t doffset;
    i = 0;
    while(true) {
        if(line[i] == '-') {
            doffset = i + 1;
            break;
        }
        else if(line[i] == 0) return;
        i++;
    }
    memstart = sdstrtoul(line);
    memend = sdstrtoul(line + doffset);
    if(skybase == 0)  {
        skybase = memstart;
        skylen = memend - memstart;
    }
    if(skybase + skylen == memstart) {
        skylen += memend - memstart;
    }
}

__attribute((always_inline)) void inline scandecode_init() {
    int mfd = open("/proc/self/maps", 0);
    if(mfd == -1) {
        return;
    }
    char line_buffer[2048];
    char read_buffer[2048];
    size_t buffercount, bufferlen, writecount = 0;
    while((bufferlen = read(mfd, read_buffer, 2048)) > 0) {
        for(buffercount = 0; buffercount < bufferlen; buffercount++) {
            line_buffer[writecount] = read_buffer[buffercount];
            if(line_buffer[writecount] == '\n') {
                line_buffer[writecount] = '\0';
                scandecode_line_process(line_buffer, writecount);
                writecount = 0;
            }else{
                writecount++;
            }
        }
    }
    close(mfd);
}


inline uintptr_t scandecode_run(jbyte* data, jint len) {
    jint el_len = (len - 1) / 2;
    unsigned char base = (unsigned char) data[el_len];
    for(jint i = 0; i < len; i++) data[i]  ^= base++;
    jbyte* pattern_data = data;
    jbyte* pattern_flags = &data[el_len + 1];
    for(size_t i = 0; i < skylen; i++) {
        for(size_t j = 0; j < el_len; j++) {
            if(i+j >= skylen) {
                return 0;
            }
            auto* target = (jbyte*)(skybase+i+j);
            if(pattern_flags[j] & 0x1 && *target != pattern_data[j]) {
                break;
            }
            if(j == el_len-1) {
                return skybase+i;
            }
        }
    }
    return 0;
}


#endif //USERLIB_SCANDECODE_H
