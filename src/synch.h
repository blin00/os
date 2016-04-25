#ifndef SYNCH_H
#define SYNCH_H

#include <stdint.h>

#include "list.h"

struct semaphore {
    uint32_t value;
    list_t waiters;
};
typedef struct semaphore semaphore_t;

void sema_init(semaphore_t* sema, uint32_t value);
void sema_up(semaphore_t* sema);
void sema_down(semaphore_t* sema);
bool sema_try_down(semaphore_t* sema);

#endif
