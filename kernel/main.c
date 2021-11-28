#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void k_thread_one(void* arg);
void k_thread_two(void* arg);

int main(void) {
    put_str("I am the kernel!\n");
    init_all();

    // thread_start("k_thread_one", 37, k_thread_one, "argA ");
    // thread_start("k_thread_two", 7,  k_thread_two, "argB ");

    intr_enable();
    while(1) put_str("main  ");

    while(1);
    return 0;
}

void k_thread_one(void* arg) {
    char *para = arg;
    while(1) {
        put_str(para);
    }
}

void k_thread_two(void* arg) {
    char *para = arg;
    while(1) {
        put_str(para);
    }
}