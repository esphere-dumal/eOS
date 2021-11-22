#include "print.h"
#include "init.h"
#include "memory.h"

int main(void) {
    put_str("I am the kernel!\n");
    init_all();
    
    void* vaddr = get_kernel_pages(3);
    put_str("\n get_kernel_pages start vaddr is: ");
    put_int((uint32_t)vaddr);
    put_char('\n');

    while(1);
    return 0;
}