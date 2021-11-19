#ifndef __LIB__STRING_H
#define __LIB__STRING_H
#include "stdint.h"

void memset(void *dst, uint8_t value, uint32_t size);
void memcpy(void *dst, void *src, uint32_t size);

// return 0 when equal, 1 when a > b, -1 when a < b
int memcmp(const void *a, const void *b, uint32_t size);

char* strcpy(char *dst, char* src);
uint32_t strlen(const char *str);
int strcmp(const char *a, const char *b);
char* strchr(const char *str, const uint8_t ch);
uint32_t strchrs(const char *str, const uint8_t ch);
char* strrchr(const char *str, const uint8_t ch);
char* strcat(char *dst, const char *src);


#endif