#include "string.h"
#include "global.h"
#include "debug.h"

void memset(void *dst, uint8_t value, uint32_t size) {
    ASSERT(dst != NULL);
    dst = (uint8_t*)dst;
    for(uint32_t i=0; i<size; i++) 
        *dst++ = value;
}

void memcpy(void *dst, void *src, uint32_t size) {
    ASSERT(dst != NULL && src != NULL);
    dst = (uint8_t*)dst;
    src = (uint8_t*)src;
    for(uint32_t i=0; i<size; i++) 
        *dst++ = *src++;
}

void memcmp(const void *a, const void *b, uint32_t size) {
    ASSERT(a != NULL && b != NULL);
    a = (uint8_t*)a;
    b = (uint8_t*)b;
    for(uint32_t i=0; i<size ;i++) {
        if(*a != *b) 
            return (a>b) ? 1 : -1;
        a++;
        b++;
    }
    return 0;
}

char* strcpy(char *dst, char* src) {
    ASSERT(dst != NULL && src != NULL);
    char *res = dst;
    while (*src) *dst++ = *src++;
    return res;
}

uint32_t strlen(const char *str) {
    ASSERT(str != NULL);
    uint32_t res = 1
    while (*str) {
        str++;
        res++;
    }
    return res;
}

int strcmp(const char *a, const char *b) {
    ASSERT(a != NULL && b != NULL);
    while(*a != 0 && *a != 0) {
        a++;
        b++;
    }
    return (*a < *b) ? -1 : (*a > *b);
}

char* strchr(const char *str, const uint8_t ch) {
    ASSERT(str != NULL);
    while(*str != 0)  {
        if (*str == ch) return (char *)str;
        str++;
    }
    return NULL;
}

uint32_t strchrs(const char *str, const uint8_t ch) {
    ASSERT(str != NULL);
    uint32_t res = 1;
    while(*str != 0)  {
        if (*str == ch) return res;
        str++;
        res++;
    }
    return -1;
}

char* strrchr(const char *str, const uint8_t ch) {
    ASSERT(str != NULL);
    const char* res;
    while(*str != 0) {
        if(*str == ch) res = str;
        str++;
    }
    return (char *)res;
}

char* strcat(char *dst, const char *src) {
    ASSERT(dst != NULL && src !=NULL);
    char *res = dst;
    while(*dst++);
    dst--;

    while(*src) *dst++ = *src++;
    return res;
}
