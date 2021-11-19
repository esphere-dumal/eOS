#ifndef __KERNEL_BITMAP_H
#define __KERNEL_BITMAP_H
#include "global.h"
#define BITMAP_MASK 1

struct  bitmap {
    uint32_t btmp_bytes_len;
    uint8_t* bits;
}

void bitmap_init(struct bitmap* btmp);

// get|set bitmap by index
bool bitmap_get(struct bitmap* btmp, uint32_t bit_idx);
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);

// alloc a series bits 
int  bitmap_alloc(struct bitmap* btmp, uint32_t cnt);

#endif