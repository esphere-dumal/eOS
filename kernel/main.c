#include "print.h"
#include "init.h"
#include "thread.h"

void k_thread_one(void* arg);

int main(void) {
    put_str("I am the kernel!\n");
    init_all();

    thread_start("k_thread_one", 37, k_thread_one, "argA ");

    while(1);
    return 0;
}

void k_thread_one(void* arg) {
    char *para = arg;
    while(1) {
        put_str(para);
    }
}