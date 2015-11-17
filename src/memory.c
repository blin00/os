#include <stddef.h>
#include <stdbool.h>

#include "io.h"
#include "string.h"
#include "memory.h"

__attribute__((aligned(4096))) static uint32_t pdt[1024];
static const size_t header_len = sizeof(malloc_header_t);
static malloc_header_t* head = NULL;

void mem_init(void* ptr, size_t size) {
    if (head || size < header_len) return;
    head = (malloc_header_t*) ptr;
    head->prev = head->next = NULL;
    head->used = false;
    head->length = size - header_len;
}

// lame first-fit allocator
void* malloc(size_t size) {
    if (!size) return NULL;
    // align size to 4 bytes
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
    // assumes that two adjacent blocks are right after each other in memory
    if (entry->next && !entry->next->used) {
        entry->length += header_len + entry->next->length;
        entry->next = entry->next->next;
    }
    if (entry->prev && !entry->prev->used) {
        entry->prev->length += header_len + entry->length;
        entry->prev->next = entry->next;
    }
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
    malloc_header_t* entry = (malloc_header_t*) ((unsigned char*) ptr - header_len);
    memcpy(buf, ptr, entry->length);
    free(ptr);
    return buf;
}

void dump_heap(void) {
    malloc_header_t* ptr = head;
    printf("*** heap dump: ***\n");
    if (!ptr) {
        printf("no entries\n");
    } else {
        while (ptr) {
            printf("entry: 0x%x, length: 0x%x%s\n", (unsigned int) ptr, ptr->length, ptr->used ? " (*)" : "");
            ptr = ptr->next;
        }
    }
}

void test_enable_paging(void) {
    asm volatile("xchg %bx, %bx");
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
