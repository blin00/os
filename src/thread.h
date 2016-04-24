#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

#include "list.h"

struct __attribute__((packed)) thread {
    list_entry_t entry;
    uint32_t esp;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t stack_base;
    bool dead;
};
typedef struct thread thread_t;

void thread_init(void);
void thread_yield(void);
void thread_yield_finish(thread_t* prev);
void thread_create(void* entry_point, void* arg);
void thread_exit(void);
thread_t* _thread_switch(thread_t* from, thread_t* to);
void _thread_entry(void);

#endif
