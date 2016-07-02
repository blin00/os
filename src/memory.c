#include <stddef.h>
#include <stdbool.h>

#include "io.h"
#include "string.h"
#include "memory.h"
#include "synch.h"

static __attribute__((aligned(4096))) uint32_t pdt[1024];
static const size_t header_len = sizeof(malloc_header_t);
static malloc_header_t* head = NULL;
static semaphore_t mem_sema;

// current limitation: can only add one contiguous block of memory to the heap
void mem_init(void* ptr, size_t size) {
    if (head || size < header_len) return;
    sema_init(&mem_sema, 1);
    head = (malloc_header_t*) ptr;
    head->prev = head->next = NULL;
    head->used = false;
    head->length = size - header_len;
}

// lame first-fit allocator
void* malloc(size_t size) {
    if (!size) return NULL;
    // round size up to 4 byte alignment
    if (size & 0b11) size = (size & ~0b11) + 4;
    sema_down(&mem_sema);
    malloc_header_t* ptr = head;
    uint8_t* result = NULL;
    while (ptr) {
        if (!ptr->used && ptr->length >= size) {
            if (size + header_len <= ptr->length) {
                malloc_header_t* split = (malloc_header_t*) ((uint8_t*) ptr + header_len + size);
                split->prev = ptr;
                split->next = ptr->next;
                ptr->next = split;
                split->used = false;
                split->length = ptr->length - size - header_len;
                ptr->length = size;
            }
            ptr->used = true;
            result = (uint8_t*) ptr + header_len;
            break;
        } else ptr = ptr->next;
    }
    sema_up(&mem_sema);
    return result;
}

void free(void* ptr) {
    if (!ptr) return;
    sema_down(&mem_sema);
    malloc_header_t* entry = (malloc_header_t*) ((uint8_t*) ptr - header_len);
    entry->used = false;
    // merge adjacent blocks
    // assumes that two adjacent blocks are right after each other in memory
    if (entry->next && !entry->next->used) {
        entry->length += header_len + entry->next->length;
        entry->next = entry->next->next;
        if (entry->next) {
            entry->next->prev = entry;
        }
    }
    if (entry->prev && !entry->prev->used) {
        entry->prev->length += header_len + entry->length;
        entry->prev->next = entry->next;
        if (entry->next) {
            entry->next->prev = entry->prev;
        }
    }
    sema_up(&mem_sema);
}

void* calloc(size_t num, size_t size) {
    void* buf = malloc(num * size);
    if (buf) memset(buf, 0, num * size);
    return buf;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    void* buf = malloc(size);
    if (!buf) return NULL;
    malloc_header_t* entry = (malloc_header_t*) ((uint8_t*) ptr - header_len);
    size_t orig_size = entry->length;
    size_t min_size = size > orig_size ? orig_size : size;
    memcpy(buf, ptr, min_size);
    free(ptr);
    return buf;
}

void dump_heap(void) {
    malloc_header_t* ptr = head;
    printf("*** heap dump: ***\n");
    if (!ptr) {
        printf("no entries\n");
    } else {
        malloc_header_t* last = NULL;
        while (ptr) {
            printf("entry: 0x%x, length: 0x%x%s\n", (uint32_t) ptr, ptr->length, ptr->used ? " (*)" : "");
            if (ptr->prev != last) printf("invalid back ptr to 0x%x\n", (uint32_t) ptr->prev);
            last = ptr;
            ptr = ptr->next;
        }
    }
}

void test_enable_paging(void) {
    memset(pdt, 0, sizeof(pdt));
    pdt[0] = 0x83;
    asm volatile(
        "mov %0, %%cr3\n\t"
        "mov %%cr4, %%eax\n\t"
        "or $0x00000010, %%eax\n\t"
        "mov %%eax, %%cr4\n\t"
        "mov %%cr0, %%eax\n\t"
        "or $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0"
        :
        : "r"(pdt)
        : "eax"
    );
}
