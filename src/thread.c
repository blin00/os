#include <stdint.h>

#include "memory.h"
#include "thread.h"
#include "io.h"

thread_t* thread_list;
thread_t* thread_cur;

// initialize the threading system
void thread_init(void) {
    thread_t* t = malloc(sizeof(thread_t));
    t->next = t->prev = t;
    thread_list = thread_cur = t;
}

void thread_yield(void) {
    thread_t* to = thread_cur->next;
    thread_t* from = thread_cur;
    if (from == to) return;
    thread_cur = to;
    _thread_switch(from, to);
    // code here may or not execute (does not execute on thread start)
}

void thread_create(void* entry, void* arg) {
    const size_t stack_size = 0x1000;
    thread_t* t = malloc(sizeof(thread_t));
    uint8_t* stack = malloc(stack_size);
    uint32_t* esp = (uint32_t*) (stack + stack_size);
    esp--; *esp = (uint32_t) arg;
    esp--; *esp = (uint32_t) thread_exit;
    esp--; *esp = (uint32_t) entry;
    t->esp = (uint32_t) esp;
    t->stack_base = (uint32_t) stack;
    // insert into linked list
    t->next = thread_list;
    t->prev = thread_list->prev;
    thread_list->prev->next = t;
    thread_list->prev = t;
}

void thread_exit(void) {
    thread_cur->prev->next = thread_cur->next;
    thread_cur->next->prev = thread_cur->prev;
    free((void*) thread_cur->stack_base);
    free(thread_cur);
    thread_yield();
}
