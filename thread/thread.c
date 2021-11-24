#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"

#define PG_SIZE 4096
#define STACK_MAGIC 0x11451411

static void kernel_thread(thread_func* function, void* func_arg) {
    function(func_arg);
}

/* create: init thread_stack
 * init  : init thread basical info
 * start : create a thread, not make it run
*/

void thread_create(struct task_struct* pthread, thread_func function, void* func_arg) {
    // intr_stack
    pthread->self_kstack -= sizeof(struct intr_stack);

    // thread_stack
    pthread->self_kstack -= sizeof(struct thread_stack);
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}

void thread_init(struct task_struct* pthread, char* name, int priority) {
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    pthread->status = RUNNING;
    pthread->priority = priority;
    pthread->self_kstack = (uint32_t*) ((uint32_t)pthread + PG_SIZE);
    pthread->stack_magic = STACK_MAGIC;
}

struct task_struct* thread_start(char* name, int priority, thread_func function, void* func_arg) {
    struct task_struct* thread = get_kernel_pages(1);

    thread_init(thread, name, priority);
    thread_create(thread, function, func_arg);

    asm volatile ( "movl %0, %%esp;  \
                    pop %%ebp; \
                    pop %%ebx; \
                    pop %%edi; \
                    pop %%esi; \
                    ret; \
                    " : : "g" (thread->self_kstack) : "memory" );
    
    return thread;
}

