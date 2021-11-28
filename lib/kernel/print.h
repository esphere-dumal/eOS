#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_asci);
void put_str(char* str);
void put_int(int num);
void set_cursor(uint32_t cursor_pos);
#endif