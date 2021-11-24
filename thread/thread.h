#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "stdint.h"

typedef void thread_func(void*);

enum task_status {
    RUNNING,
    READY,
    BLOCKED,
    WAITING,
    HANGING,
    DIED
};

struct intr_stack {
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;

    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t err_code;
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};  

struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    
    void (*eip) (thread_func* func, void* func_arg);

    void (*unused_retaddr);

    thread_func* function;
    void* func_arg;
};

struct task_struct {
    uint32_t* self_kstack;
    enum task_status status;
    uint8_t priority;
    char name[16];
    uint32_t stack_magic;
};

void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void thread_init(struct task_struct* pthread, char* name, int priority);
struct task_struct* thread_start(char* name, int priority, thread_func function, void* func_arg);
#endif
