#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

struct virtual_addr {
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

enum pool_flag {
    PF_KERNEL = 1, // kernel pool flag
    PF_USER   = 2  // user   pool flag
};

#define PG_P_1 1    // xx1
#define PG_P_0 0    // xx0 
#define PG_RW_R 0   // x0x
#define PG_RW_W 2   // x1x
#define PG_US_S 0   // 0xx
#define PG_US_U 4   // 1xx
 

extern struct pool kernel_pool, user_pool;

void mem_init();

#endif