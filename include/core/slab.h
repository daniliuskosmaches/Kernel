#ifndef SLAB_H
#define SLAB_H

#include "stdint.h"
#include "stddef.h"

// Инициализация slab allocator
void slab_init(void);

// Быстрое выделение памяти через slab allocator
void* kmalloc_fast(uint32_t size);

// Быстрое освобождение памяти через slab allocator
void kfree_fast(void* ptr);

#endif // SLAB_H