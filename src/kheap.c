// src/kheap.c
// Файл содержит минимально необходимые заглушки для успешной линковки.
// src/kheap.c - ПОЛНОЦЕННАЯ РЕАЛИЗАЦИЯ КУЧИ

#include "kheap.h"
#include "vmm.h"
#include "pmm.h"
#include "string.h"

// Диапазон виртуальных адресов для кучи
#define KHEAP_START     0xC0400000  // Начало кучи (после 4MB identity mapping)
#define KHEAP_INITIAL   0xC0400000
#define KHEAP_MAX       0xC0C00000  // Максимум 8MB кучи
#define HEAP_MIN_SIZE   0x100000    // Начальный размер 1MB

// Заголовок блока памяти
typedef struct heap_block {
    uint32_t size;              // Размер блока (включая заголовок)
    uint32_t magic;             // Магическое число для проверки целостности
    struct heap_block *next;    // Следующий свободный блок
    uint8_t is_free;            // 1 = свободен, 0 = занят
} heap_block_t;

#define HEAP_MAGIC 0x48454150   // "HEAP" в ASCII
#define BLOCK_HEADER_SIZE sizeof(heap_block_t)

// Глобальные переменные кучи
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static uint32_t heap_max = KHEAP_MAX;
static heap_block_t *free_list = 0;

// Выравнивание на 16 байт для лучшей производительности
#define ALIGN_SIZE(size) (((size) + 0xF) & ~0xF)

// ============================================================
// ИНИЦИАЛИЗАЦИЯ КУЧИ
// ============================================================
void kheap_init(void) {
    heap_start = KHEAP_START;
    heap_end = KHEAP_START + HEAP_MIN_SIZE;

    // Отображаем начальную память кучи через VMM
    for (uint32_t addr = heap_start; addr < heap_end; addr += PAGE_SIZE) {
        uint32_t phys = (uint32_t)pmm_alloc_page();
        if (phys == 0) {
            // Критическая ошибка - не хватает физической памяти
            return;
        }
        vmm_map_page(addr, phys, PAGE_PRESENT | PAGE_RW);
    }

    // Инициализируем первый свободный блок
    free_list = (heap_block_t*)heap_start;
    free_list->size = HEAP_MIN_SIZE - BLOCK_HEADER_SIZE;
    free_list->magic = HEAP_MAGIC;
    free_list->next = 0;
    free_list->is_free = 1;
}

// ============================================================
// РАСШИРЕНИЕ КУЧИ
// ============================================================
static int expand_heap(uint32_t new_size) {
    if (heap_end + new_size > heap_max) {
        return 0; // Достигнут максимум
    }

    uint32_t old_end = heap_end;
    uint32_t new_end = heap_end + new_size;

    // Отображаем новые страницы
    for (uint32_t addr = old_end; addr < new_end; addr += PAGE_SIZE) {
        uint32_t phys = (uint32_t)pmm_alloc_page();
        if (phys == 0) {
            return 0; // Не хватает физической памяти
        }
        vmm_map_page(addr, phys, PAGE_PRESENT | PAGE_RW);
    }

    heap_end = new_end;

    // Создаем новый свободный блок в расширенной области
    heap_block_t *new_block = (heap_block_t*)old_end;
    new_block->size = new_size - BLOCK_HEADER_SIZE;
    new_block->magic = HEAP_MAGIC;
    new_block->is_free = 1;
    new_block->next = 0;

    // Добавляем в список свободных блоков
    if (free_list == 0) {
        free_list = new_block;
    } else {
        heap_block_t *current = free_list;
        while (current->next != 0) {
            current = current->next;
        }
        current->next = new_block;
    }

    return 1;
}

// ============================================================
// ВЫДЕЛЕНИЕ ПАМЯТИ (kmalloc)
// ============================================================
void* kmalloc(size_t size) {
    if (size == 0) return 0;

    // Выравниваем размер
    size = ALIGN_SIZE(size);

    // Ищем подходящий свободный блок (First Fit)
    heap_block_t *current = free_list;
    heap_block_t *prev = 0;

    while (current != 0) {
        // Проверяем магическое число
        if (current->magic != HEAP_MAGIC) {
            // Повреждение кучи!
            return 0;
        }

        if (current->is_free && current->size >= size) {
            // Нашли подходящий блок

            // Если блок слишком большой, разделяем его
            if (current->size >= size + BLOCK_HEADER_SIZE + 16) {
                heap_block_t *new_block = (heap_block_t*)((uint32_t)current + BLOCK_HEADER_SIZE + size);
                new_block->size = current->size - size - BLOCK_HEADER_SIZE;
                new_block->magic = HEAP_MAGIC;
                new_block->is_free = 1;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->is_free = 0;

            // Возвращаем указатель на данные (после заголовка)
            return (void*)((uint32_t)current + BLOCK_HEADER_SIZE);
        }

        prev = current;
        current = current->next;
    }

    // Не нашли подходящий блок - расширяем кучу
    uint32_t expand_size = (size + BLOCK_HEADER_SIZE > 0x10000) ?
                           size + BLOCK_HEADER_SIZE : 0x10000;
    expand_size = ALIGN_SIZE(expand_size);

    if (!expand_heap(expand_size)) {
        return 0; // Не удалось расширить
    }

    // Пробуем снова
    return kmalloc(size);
}

// ============================================================
// ОСВОБОЖДЕНИЕ ПАМЯТИ (kfree)
// ============================================================
void kfree(void* ptr) {
    if (ptr == 0) return;

    // Получаем заголовок блока
    heap_block_t *block = (heap_block_t*)((uint32_t)ptr - BLOCK_HEADER_SIZE);

    // Проверяем магическое число
    if (block->magic != HEAP_MAGIC) {
        // Повреждение кучи или некорректный указатель
        return;
    }

    // Помечаем блок как свободный
    block->is_free = 1;

    // ОБЪЕДИНЕНИЕ СМЕЖНЫХ СВОБОДНЫХ БЛОКОВ (Coalescing)
    heap_block_t *current = free_list;

    while (current != 0) {
        if (current->is_free && current->next != 0 && current->next->is_free) {
            // Объединяем два смежных свободных блока
            if ((uint32_t)current + BLOCK_HEADER_SIZE + current->size == (uint32_t)current->next) {
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            }
        }
        current = current->next;
    }
}

// ============================================================
// ПЕРЕВЫДЕЛЕНИЕ ПАМЯТИ (krealloc)
// ============================================================
void* krealloc(void* ptr, size_t new_size) {
    if (ptr == 0) {
        return kmalloc(new_size);
    }

    if (new_size == 0) {
        kfree(ptr);
        return 0;
    }

    // Получаем заголовок старого блока
    heap_block_t *block = (heap_block_t*)((uint32_t)ptr - BLOCK_HEADER_SIZE);

    if (block->magic != HEAP_MAGIC) {
        return 0; // Некорректный указатель
    }

    // Если новый размер меньше или равен текущему, возвращаем тот же указатель
    if (new_size <= block->size) {
        return ptr;
    }

    // Выделяем новый блок
    void* new_ptr = kmalloc(new_size);
    if (new_ptr == 0) {
        return 0;
    }

    // Копируем данные
    memcpy(new_ptr, ptr, block->size);

    // Освобождаем старый блок
    kfree(ptr);

    return new_ptr;
}

// ============================================================
// ВЫДЕЛЕНИЕ ВЫРОВНЕННОЙ ПАМЯТИ (для Page Directory и т.д.)
// ============================================================
void* kmalloc_aligned(size_t size, uint32_t alignment) {
    // Выделяем больше памяти для выравнивания
    void* ptr = kmalloc(size + alignment + sizeof(void*));
    if (ptr == 0) return 0;

    // Вычисляем выровненный адрес
    uint32_t aligned_addr = ((uint32_t)ptr + sizeof(void*) + alignment - 1) & ~(alignment - 1);

    // Сохраняем оригинальный указатель перед выровненным адресом
    *((void**)(aligned_addr - sizeof(void*))) = ptr;

    return (void*)aligned_addr;
}

void kfree_aligned(void* ptr) {
    if (ptr == 0) return;

    // Получаем оригинальный указатель
    void* original = *((void**)((uint32_t)ptr - sizeof(void*)));
    kfree(original);
}