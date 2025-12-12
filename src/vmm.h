// src/vmm.h
#ifndef VMM_H
#define VMM_H

#include "stdint.h"
#include "stddef.h" // Для size_t (если используется в других функциях VMM)

// Размер страницы
#define PAGE_SIZE 4096

// Флаги записей страниц (используются в vmm_map_page)
#define PAGE_PRESENT 0x01
#define PAGE_RW      0x02
#define PAGE_USER    0x04

// ----------------------------------------------------
// 1. Запись в Таблице Страниц (Page Table Entry, PTE)
// ----------------------------------------------------
typedef struct {
    uint32_t present     : 1;  // [0] 1 = Страница присутствует в памяти
    uint32_t rw          : 1;  // [1] 1 = Чтение/Запись, 0 = Только чтение
    uint32_t user        : 1;  // [2] 1 = Уровень привилегий Ring 3 (Пользователь)
    uint32_t pwt         : 1;  // [3] Write-Through
    uint32_t pcd         : 1;  // [4] Отключить кэш
    uint32_t accessed    : 1;  // [5] Страница была использована (устанавливается CPU)
    uint32_t dirty       : 1;  // [6] Страница была записана (устанавливается CPU)
    uint32_t pat         : 1;  // [7] Page Attribute Table
    uint32_t global      : 1;  // [8] Глобальная страница (TLB)
    uint32_t available   : 3;  // [9-11] Доступно для использования OS
    uint32_t frame_addr  : 20; // [12-31] 20 старших бит адреса физического кадра
} page_table_entry_t;


// ----------------------------------------------------
// 2. Запись в Каталоге Страниц (Page Directory Entry, PDE)
// ----------------------------------------------------
typedef struct {
    uint32_t present     : 1;  // [0] 1 = Таблица страниц присутствует
    uint32_t rw          : 1;  // [1] Права для всей таблицы
    uint32_t user        : 1;  // [2] Привилегии для всей таблицы
    uint32_t pwt         : 1;
    uint32_t pcd         : 1;
    uint32_t accessed    : 1;  // [5] Таблица была использована
    uint32_t reserved    : 1;
    uint32_t page_size   : 1;  // [7] 0 = 4КБ страницы (мы используем)
    uint32_t global      : 1;
    uint32_t available   : 3;
    uint32_t table_addr  : 20; // [12-31] 20 старших бит адреса Таблицы Страниц
} page_directory_entry_t;

typedef struct {
    page_table_entry_t entries[1024]; // 1024 записи
} page_table_t; // <--- Этот тип не был корректно виден в vmm.c


// ----------------------------------------------------
// 4. Структура Каталога Страниц (PDT)
// ----------------------------------------------------
// Это структура, которую мы выделяем через pmm_alloc_page()
typedef struct {
    page_directory_entry_t entries[1024]; // 1024 записи
} page_directory_t;

// ----------------------------------------------------
// 3. Структура Каталога Страниц (PDT)
// ----------------------------------------------------
// Это структура, которую мы выделяем через pmm_alloc_page().
// Поле 'entries' делает код в vmm.c более читаемым (e.g., pde->entries[i])



// ----------------------------------------------------
// Объявления функций VMM
// ----------------------------------------------------
void vmm_init(void);
void vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void vmm_unmap_page(uint32_t virt_addr); // Опционально, но полезно
void vmm_switch_directory(page_directory_t *dir);


#endif