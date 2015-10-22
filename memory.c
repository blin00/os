#include <stddef.h>
#include <stdbool.h>

#include "memory.h"
#include "io.h"

static const size_t header_len = sizeof(malloc_header_t);
static malloc_header_t* head = NULL;

void init_mem(void* ptr, size_t size) {
    if (head || size < header_len) return;
    head = (malloc_header_t*) ptr;
    head->prev = head->next = NULL;
    head->used = false;
    head->length = size - header_len;
}

// lame first-fit allocator
void* malloc(size_t size) {
    if (size & 0b11) size = (size & ~0b11) + 4;
    malloc_header_t* ptr = head;
    while (ptr) {
        if (!ptr->used && ptr->length >= size) {
            if (size + header_len <= ptr->length) {
                malloc_header_t* split = (malloc_header_t*) ((unsigned char*) ptr + header_len + size);
                split->prev = ptr;
                split->next = ptr->next;
                ptr->next = split;
                split->used = false;
                split->length = ptr->length - size - header_len;
                ptr->length = size;
            }
            ptr->used = true;
            return (unsigned char*) ptr + header_len;
        } else ptr = ptr->next;
    }
    return NULL;
}

void free(void* ptr) {
    if (!ptr) return;
    malloc_header_t* entry = (malloc_header_t*) ((unsigned char*) ptr - header_len);
    entry->used = false;
    // merge adjacent blocks
    if (entry->next && !entry->next->used) {
        entry->length += header_len + entry->next->length;
        entry->next = entry->next->next;
    }
    if (entry->prev && !entry->prev->used) {
        entry->prev->length += header_len + entry->length;
        entry->prev->next = entry->next;
    }
}

void dump_heap(void) {
    malloc_header_t* ptr = head;
    printf("\n*** heap dump: ***");
    if (!ptr) {
        printf("no entries\n");
    } else {
        while (ptr) {
            printf("entry: 0x%x, length: 0x%x%s\n", (unsigned int) ptr, ptr->length, ptr->used ? " (*)" : "");
            ptr = ptr->next;
        }
    }
}
