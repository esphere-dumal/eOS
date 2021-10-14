#include "print.h"
#include "init.h"
int main(void) {
    put_str("I am kernel!\n");
    put_int(114514);
    init_all();
    asm volatile("sti");
    while(1);
    return 0;
}