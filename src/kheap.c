#include "../include/core/slab.h" // Твой скоростной аллокатор здесь?
#include <stdint.h>
#include <stddef.h>

// Эти переменные часто приходят из pmm.c или linker.ld
extern uint32_t placement_address;

typedef struct header {
    struct header *next;
    size_t size;
    uint8_t is_free;
} header_t;

static header_t *free_list = NULL;
static uint32_t heap_end = 0;

void kheap_init(uint32_t start_addr, uint32_t end_addr) {
    free_list = (header_t *)start_addr;
    free_list->size = (end_addr - start_addr) - sizeof(header_t);
    free_list->next = NULL;
    free_list->is_free = 1;
    heap_end = end_addr;
}

// Базовый kmalloc (First Fit)
void *kmalloc(size_t size) {
    header_t *curr = free_list;

    while (curr) {
        if (curr->is_free && curr->size >= size) {
            // Если блок сильно больше нужного, отрезаем лишнее (Split)
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

    return NULL; // Out of memory
}

// Тот самый "быстрый" аллокатор, который ты звал в task.c
void *kmalloc_fast(size_t size) {
    // В простейшем случае это обертка над kmalloc,
    // но здесь можно добавить кэширование мелких объектов (Slab)
    return kmalloc(size);
}

void kfree(void *ptr) {
    if (!ptr) return;

    header_t *header = (header_t *)((uint8_t *)ptr - sizeof(header_t));
    header->is_free = 1;

    // ОПТИМИЗАЦИЯ: Слияние соседних свободных блоков (Coalescing)
    header_t *curr = free_list;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(header_t) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}