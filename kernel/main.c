#include "print.h"
#include "init.h"
#include "debug.h"

int main(void) {
    put_str("I am the kernel!\n");
    init_all();
//     asm volatile("sti");
    put_str("before assert\n");
    ASSERT(1==114514);
    put_str("after assert\n");
    while(1);
    return 0;
}