// src/kheap.h
#ifndef KHEAP_H
#define KHEAP_H

#include "stdint.h"
#include "stddef.h"
#include "vmm.h"
#include "pmm.h"

// ============================================================
// ИНТЕРФЕЙС МЕНЕДЖЕРА КУЧИ ЯДРА
// ============================================================

/**
 * Инициализирует кучу ядра
 * Должна быть вызвана после инициализации PMM и VMM
 */
extern void kheap_init(void);

/**
 * Выделяет блок памяти указанного размера
 * @param size: Размер в байтах
 * @return: Указатель на выделенный блок или NULL при ошибке
 */
extern void* kmalloc(size_t size);

/**
 * Освобождает ранее выделенный блок памяти
 * @param p: Указатель на блок
 */
extern void kfree(void* p);

/**
 * Перевыделяет блок памяти с новым размером
 * @param p: Указатель на старый блок
 * @param new_size: Новый размер
 * @return: Указатель на новый блок или NULL
 */
extern void* krealloc(void* p, size_t new_size);

/**
 * Выделяет выровненный блок памяти
 * @param size: Размер в байтах
 * @param alignment: Выравнивание (должно быть степенью 2)
 * @return: Указатель на выровненный блок или NULL
 */
extern void* kmalloc_aligned(size_t size, uint32_t alignment);

/**
 * Освобождает выровненный блок
 * @param ptr: Указатель на блок
 */
extern void kfree_aligned(void* ptr);

#endif // KHEAP_H
