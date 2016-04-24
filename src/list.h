#ifndef LIST_H
#define LIST_H

#include <stdint.h>

#define container_of(ptr, type, field) ((type*) ((uint8_t*) (ptr) - offsetof(type, field)))

struct list_entry {
    struct list_entry* prev;
    struct list_entry* next;
};
typedef struct list_entry list_entry_t;

struct list
{
    list_entry_t head;
    list_entry_t tail;
};
typedef struct list list_t;

void list_init(list_t* list);
void list_push_front(list_t* list, list_entry_t* entry);
void list_push_back(list_t* list, list_entry_t* entry);
void list_remove(list_entry_t* entry);

#endif
