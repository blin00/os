#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "memory.h"
#include "list.h"
#include "interrupt.h"
#include "thread.h"

list_t thread_ready_list;
thread_t* thread_cur;   // not in ready list
thread_t* thread_idle;

static void thread_idle_proc(void);

// initialize the threading system
thread_t* thread_init(void) {
    list_init(&thread_ready_list);
    thread_t* t = malloc(sizeof(thread_t));
    t->state = READY;
    thread_cur = t;
    thread_idle = thread_create(thread_idle_proc, NULL);
    return t;
}

thread_t* thread_create(void* entry_point, void* arg) {
    const size_t stack_size = 0x4000;   // TODO: figure out why we need so much stack
    thread_t* t = malloc(sizeof(thread_t));
    uint8_t* stack = malloc(stack_size);
    uint32_t* esp = (uint32_t*) (stack + stack_size);
    esp--; *esp = (uint32_t) arg;           // argument for entry_point
    esp--; *esp = (uint32_t) thread_exit;   // return address for entry_point
    esp--; *esp = (uint32_t) entry_point;   // return address for _thread_entry
    esp--; *esp = (uint32_t) _thread_entry; // return for _thread_switch
    t->esp = (uint32_t) esp;
    t->stack_base = (uint32_t) stack;
    t->state = READY;
    bool old = int_disable();
    list_push_back(&thread_ready_list, &t->entry);
    int_set(old);
    return t;
}

static void thread_idle_proc() {
    while (true) {
        thread_block();
        asm volatile("hlt");
    }
}

void thread_yield(void) {
    bool old = int_disable();
    thread_t* prev = NULL;
    if (thread_cur != thread_idle && thread_cur->state == READY)
        list_push_back(&thread_ready_list, &thread_cur->entry);
    thread_t* to;
    if (list_is_empty(&thread_ready_list)) {
        to = thread_idle;
    } else {
        to = container_of(list_pop_front(&thread_ready_list), thread_t, entry);
    }
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
    if (prev && prev->state == DEAD) {
        free((void*) prev->stack_base);
        free(prev);
    }
}

void thread_block() {
    bool old = int_disable();
    thread_cur->state = BLOCKED;
    thread_yield();
    int_set(old);
}

void thread_unblock(thread_t* thread) {
    bool old = int_disable();
    thread->state = READY;
    list_push_back(&thread_ready_list, &thread->entry);
    int_set(old);
}

void thread_exit(void) {
    thread_cur->state = DEAD;
    thread_yield();
}
