#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

#include "list.h"

enum thread_state {READY, BLOCKED, DEAD};
typedef enum thread_state thread_state_t;

struct __attribute__((packed)) thread {
    list_entry_t entry;
    uint32_t esp;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t stack_base;
    thread_state_t state;
};
typedef struct thread thread_t;

extern thread_t* thread_cur;

thread_t* thread_init(void);
thread_t* thread_create(void* entry_point, void* arg);
void thread_yield(void);
void thread_yield_finish(thread_t* prev);
void thread_block(void);
void thread_unblock(thread_t* thread);
void thread_exit(void);
thread_t* _thread_switch(thread_t* from, thread_t* to);
void _thread_entry(void);

#endif
