#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stddef.h>

struct malloc_header {
    struct malloc_header* prev;
    struct malloc_header* next;
    size_t length;
    bool used;
};
typedef struct malloc_header malloc_header_t;

void init_mem(void* ptr, size_t size);
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
void dump_heap(void);

#endif
