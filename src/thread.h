#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

struct __attribute__((packed)) thread {
    struct thread* next;
    uint32_t esp;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};
typedef struct thread thread_t;

void thread_init(void);
void thread_yield(void);
void thread_create(void* entry);
void _thread_switch(thread_t* from, thread_t* to);

#endif
