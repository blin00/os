#include <stdint.h>

#include "memory.h"
#include "thread.h"
#include "list.h"
#include "interrupt.h"

list_t thread_ready_list;
thread_t* thread_cur;   // not in ready list

// initialize the threading system
void thread_init(void) {
    list_init(&thread_ready_list);
    thread_t* t = malloc(sizeof(thread_t));
    t->dead = false;
    thread_cur = t;
}

void thread_yield(void) {
    bool old = int_disable();
    thread_t* prev = NULL;
    if (!thread_cur->dead)
        list_push_back(&thread_ready_list, &thread_cur->entry);
    // assume there's always a thread ready to run
    list_entry_t* entry_next = thread_ready_list.head.next;
    list_remove(entry_next);
    thread_t* to = container_of(entry_next, thread_t, entry);
    thread_t* from = thread_cur;
    if (from != to) {
        thread_cur = to;
        prev = _thread_switch(from, to);
    }
    // if the thread we're switching to is new, this code isn't executed
    // _thread_entry calls thread_yield_finish instead (and then enables interrupts)
    thread_yield_finish(prev);
    int_set(old);
}

void thread_yield_finish(thread_t* prev) {
    if (prev && prev->dead) {
        free((void*) prev->stack_base);
        free(prev);
    }
}

void thread_create(void* entry_point, void* arg) {
    const size_t stack_size = 0x1000;
    thread_t* t = malloc(sizeof(thread_t));
    uint8_t* stack = malloc(stack_size);
    uint32_t* esp = (uint32_t*) (stack + stack_size);
    esp--; *esp = (uint32_t) arg;
    esp--; *esp = (uint32_t) thread_exit;
    esp--; *esp = (uint32_t) entry_point;
    esp--; *esp = (uint32_t) _thread_entry;
    t->esp = (uint32_t) esp;
    t->stack_base = (uint32_t) stack;
    t->dead = false;
    bool old = int_disable();
    list_push_back(&thread_ready_list, &t->entry);
    int_set(old);
}

void thread_exit(void) {
    thread_cur->dead = true;
    thread_yield();
}
