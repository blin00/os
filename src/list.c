#include <stddef.h>
#include <stdbool.h>

#include "list.h"

void list_init(list_t* list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

bool list_is_empty(list_t* list) {
    return list->head.next == &list->tail;
}

void list_push_front(list_t* list, list_entry_t* entry) {
    list_entry_t* old_front = list->head.next;
    entry->prev = &list->head;
    entry->next = old_front;
    old_front->prev = entry;
    list->head.next = entry;
}

void list_push_back(list_t* list, list_entry_t* entry) {
    list_entry_t* old_back = list->tail.prev;
    entry->prev = old_back;
    entry->next = &list->tail;
    old_back->next = entry;
    list->tail.prev = entry;
}

list_entry_t* list_pop_front(list_t* list) {
    list_entry_t* front = list->head.next;
    list_remove(front);
    return front;
}

list_entry_t* list_pop_back(list_t* list) {
    list_entry_t* back = list->tail.prev;
    list_remove(back);
    return back;
}

void list_remove(list_entry_t* entry) {
    list_entry_t* prev = entry->prev;
    list_entry_t* next = entry->next;
    prev->next = next;
    next->prev = prev;
    entry->prev = entry->next = entry;
}
