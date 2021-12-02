#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "memory.h"
#include "thread.h"
#include "../device/timer.h"
#include "../device/console.h"

void init_all() {
    put_str("init_all\n");
    idt_init();
    mem_init();
    threads_init();
    timer_init();
    console_init();
}