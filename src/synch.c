#include <stdint.h>

#include "thread.h"
#include "interrupt.h"
#include "list.h"
#include "synch.h"

void sema_init(semaphore_t* sema, uint32_t value) {
    sema->value = value;
    list_init(&sema->waiters);
}

void sema_up(semaphore_t* sema) {
    bool old = int_disable();
    sema->value++;
    if (!list_is_empty(&sema->waiters)) {
        thread_t* waiter = container_of(list_pop_front(&sema->waiters), thread_t, entry);
        thread_unblock(waiter);
    }
    int_set(old);
}

void sema_down(semaphore_t* sema) {
    bool old = int_disable();
    while (sema->value == 0) {
        list_push_back(&sema->waiters, &thread_cur->entry);
        thread_block();
    }
    sema->value--;
    int_set(old);
}

bool sema_try_down(semaphore_t* sema) {
    bool old = int_disable();
    bool success = false;
    if (sema->value > 0) {
        sema->value--;
        success = true;
    }
    int_set(old);
    return success;
}
