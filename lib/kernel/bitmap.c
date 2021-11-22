#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

void bitmap_init(struct bitmap* btmp) {
    memset(btmp->bits, 0, btmp->btmp_bytes_len);
}

bool bitmap_get(struct bitmap* btmp, uint32_t bit_idx) {
    uint32_t byte_index  = bit_idx / 8;
    uint32_t byte_offset = bit_idx % 8;

    return (btmp -> bits[byte_index] & (BITMAP_MASK << byte_offset));
}

void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value) {
    ASSERT(value == 0 || value == 1);
    uint32_t byte_index  = bit_idx / 8;
    uint32_t byte_offset = bit_idx % 8;

    if(value) 
        btmp->bits[byte_index] |=  (BITMAP_MASK << byte_offset);
    else 
        btmp->bits[byte_index] &=  ~(BITMAP_MASK << byte_offset);
}


int bitmap_alloc(struct bitmap* btmp, uint32_t cnt) {
    uint32_t byte_index = 0;
    while ( (btmp->bits[byte_index] == 0xff) 
         && (byte_index < btmp->btmp_bytes_len))
        byte_index++;
    
    // failed
    if(byte_index >= btmp->btmp_bytes_len)
        return -1;

    // find the first free bit
    uint32_t bit_idx = 0;
    while(btmp->bits[bit_idx] & (uint8_t)(BITMAP_MASK << bit_idx))
        bit_idx++;
    uint32_t bit_start_idx = byte_index *8 + bit_idx;

    // check the rest bit for a series of free bits
    uint32_t next_bit = bit_start_idx + 1;
    uint32_t count = 1;
    for(uint32_t i=bit_start_idx; i<(btmp->btmp_bytes_len*8); i++) {
        if(!bitmap_get(btmp, next_bit)) count++;
        else count = 0;
        if(count == cnt) return next_bit - cnt + 1;
        next_bit++;
    }
    return -1;
}