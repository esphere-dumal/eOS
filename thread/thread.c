#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "interrupt.h"
#include "list.h"
#include "debug.h"

#define PG_SIZE 4096
#define STACK_MAGIC 0x11451411

extern switch_to(struct task_struct* cur, struct task_struct* next);

struct task_struct* main_thread;
struct list thread_ready_list;
struct list thread_all_list;
static struct list_elem* thread_tag;

static void kernel_thread(thread_func* function, void* func_arg) {
    intr_enable();
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
    if(pthread == main_thread) pthread->status = RUNNING;
    else pthread->status = READY;
    pthread->priority = priority;
    pthread->ticks = priority;
    pthread->elapsed_ticks = 0;
    pthread->pagedir = NULL;
    pthread->self_kstack = (uint32_t*) ((uint32_t)pthread + PG_SIZE);
    pthread->stack_magic = STACK_MAGIC;
}

struct task_struct* thread_start(char* name, int priority, thread_func function, void* func_arg) {
    struct task_struct* thread = get_kernel_pages(1);
    put_str("thread address: ");
    put_int(thread);
    put_str("\n");

    thread_init(thread, name, priority);
    thread_create(thread, function, func_arg);

    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));

    list_append(&thread_ready_list, &thread->general_tag);
    list_append(&thread_all_list, &thread->all_list_tag);

    // asm volatile ( "movl %0, %%esp;  \
    //                 pop %%ebp; \
    //                 pop %%ebx; \
    //                 pop %%edi; \
    //                 pop %%esi; \
    //                 ret; \
    //                 " : : "g" (thread->self_kstack) : "memory" );
    
    return thread;
}

struct task_struct* running_thread() {
    uint32_t esp;
    asm("mov %%esp, %0" : "=g" (esp));
    return (struct task_struct*)(esp & 0xfffff000);
}

static make_main_thread() {
    main_thread = running_thread();
    thread_init(main_thread, "main", 31);

    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

void schedule() {
    ASSERT(intr_get_status() == INTR_OFF);
    struct task_struct* cur = running_thread(); 
    if (cur->status == RUNNING) { 
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list, &cur->general_tag);

        cur->ticks = cur->priority;
        cur->status = READY;
    }
    else { }

    ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL;
    thread_tag = list_pop(&thread_ready_list);
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = RUNNING;
    switch_to(cur, next);
}

void threads_init() {
    put_str("threads_init start\n");
    list_init(&thread_all_list);
    list_init(&thread_ready_list);
    make_main_thread();
    put_str("threads_init done\n");
}


void thread_block(enum task_status_ stat) {
    ASSERT ( (stat == BLOCKED) || (stat == WAITING) || (stat == HANGING) );
    enum intr_status old_status = intr_disable();
    struct task_struct* cur_thread = running_thread();
    cur_thread-> status = stat;
    schedule();
    intr_set_status(old_status);
}

void thread_unblock(struct task_struct* pthread) {
    enum intr_status old_status = intr_disable();
    ASSERT ( (pthread->status == BLOCKED) || (pthread->status == WAITING) || (pthread->status == HANGING) );
    if(pthread->status != READY) {
        if(elem_find(&thread_ready_list, &pthread->general_tag)) 
            PANIC("Thread_unblock: blocked thread in ready list");

        list_push(&thread_ready_list, &pthread->general_tag);
        pthread->status = READY;
    }
    intr_set_status(old_status);
}