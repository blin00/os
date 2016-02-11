#include <stdint.h>

#include "memory.h"
#include "thread.h"

thread_t* thread_list;
thread_t* thread_cur;

void thread_init(void) {
    thread_t* t = malloc(sizeof(thread_t));
    t->next = NULL;
    thread_list = thread_cur = t;
}

void thread_yield(void) {
    thread_t* to = thread_cur->next;
    if (!to) to = thread_list;
    thread_t* from = thread_cur;
    thread_cur = to;
    _thread_switch(from, to);
    // code here may not execute
}

void thread_create(void* entry) {
    const size_t stack_size = 0x1000;
    thread_t* t = malloc(sizeof(thread_t));
    uint8_t* stack = malloc(stack_size);
    uint32_t* esp = (uint32_t*) (stack + stack_size);
    esp--; *esp = (uint32_t) t;
    esp--; *esp = (uint32_t) entry;
    t->esp = (uint32_t) esp;
    t->next = thread_list;
    thread_list = t;
}
