#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stddef.h>

#define ALIGN(size) (((size) + 3) & ~3)

typedef struct header {
    struct header *next;
    size_t size;
    uint8_t is_free;
} header_t;

static header_t *free_list = NULL;

void kheap_init(uint32_t start_addr, uint32_t end_addr) {
    start_addr = ALIGN(start_addr);
    free_list = (header_t *)start_addr;
    free_list->size = (end_addr - start_addr) - sizeof(header_t);
    free_list->next = NULL;
    free_list->is_free = 1;
}

void *kmalloc(size_t size) {
    size = ALIGN(size);
    header_t *curr = free_list;
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size > size + sizeof(header_t) + 4) {
                header_t *next_block = (header_t *)((uint8_t *)curr + sizeof(header_t) + size);
                next_block->size = curr->size - size - sizeof(header_t);
                next_block->is_free = 1;
                next_block->next = curr->next;
                curr->size = size;
                curr->next = next_block;
            }
            curr->is_free = 0;
            return (void *)((uint8_t *)curr + sizeof(header_t));
        }
        curr = curr->next;
    }
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr) return;
    header_t *header = (header_t *)((uint8_t *)ptr - sizeof(header_t));
    header->is_free = 1;
    header_t *curr = free_list;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(header_t) + curr->next->size;
            curr->next = curr->next->next;
        } else curr = curr->next;
    }
}