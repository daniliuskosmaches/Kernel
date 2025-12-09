//
// Created by nazar on 09.12.25.
//

// src/vmm.c
#include "string.h"
#include "vmm.h"
#include "pmm.h"
#include "string.h"
// Для pmm_alloc_page
 // Для отладки (если у вас есть terminal_write_string)

// ----------------------------------------------------
// Глобальные переменные VMM
// ----------------------------------------------------
// Текущий каталог страниц ядра
page_directory_t *current_page_directory = 0;

// Флаги по умолчанию для записей (Present, R/W, Kernel)
#define PAGE_PRESENT 0x01
#define PAGE_RW      0x02
#define PAGE_USER    0x04 // Мы пока используем только Kernel (0)

// ----------------------------------------------------
// 1. vmm_map_page: Отображение Виртуального Адреса на Физический
// ----------------------------------------------------
void vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {

    // 1. Выравнивание адресов на границу 4КБ
    virt_addr &= 0xFFFFF000;
    phys_addr &= 0xFFFFF000;

    // Индексы
    uint32_t pd_index = virt_addr >> 22;      // Индекс в каталоге страниц (биты 22-31)
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF; // Индекс в таблице страниц (биты 12-21)

    // Получаем записи
    page_directory_entry_t *pde = &current_page_directory->entries[pd_index];

    // 2. Если таблица страниц (PT) еще не существует, создаем ее
    if (!(pde->present)) {

        // Выделяем кадр для новой таблицы страниц (4КБ)
        uint32_t new_table_phys_addr = (uint32_t)pmm_alloc_page();

        // Проверяем, удалось ли выделить память
        if (new_table_phys_addr == 0) {
            // terminal_write_string("VMM ERROR: No memory for Page Table!");
            return;
        }





        // Заполняем запись в каталоге страниц (PDE)
        pde->present = 1;
        pde->rw = 1;
        pde->user = 0;
        pde->table_addr = new_table_phys_addr >> 12; // 20 старших бит адреса
    }

    // 3. Получаем адрес таблицы страниц (PT)
    // Адрес в PDT хранится как 20 старших бит. Добавляем 12 нулей.
    page_table_t *pt = (page_table_t*)(pde->table_addr << 12);

    // 4. Заполняем запись в таблице страниц (PTE)
    page_table_entry_t *pte = &pt->entries[pt_index];

    // Проверяем, не занята ли страница
    if (pte->present) {
        // terminal_write_string("VMM WARNING: Page already mapped!");
        // Здесь можно реализовать unmap или обработку ошибки
    }

    pte->present = (flags & PAGE_PRESENT) ? 1 : 0;
    pte->rw = (flags & PAGE_RW) ? 1 : 0;
    pte->user = (flags & PAGE_USER) ? 1 : 0;
    pte->frame_addr = phys_addr >> 12; // 20 старших бит физического адреса

    // Сброс TLB для этой записи (если пейджинг уже включен)
    // __asm__ __volatile__("invlpg (%0)" ::"r" (virt_addr));
}


// ----------------------------------------------------
// 2. vmm_init: Инициализация и Включение Пейджинга
// ----------------------------------------------------
void vmm_init(void) {
    // 1. Выделяем память для нового каталога страниц (PDT)
    page_directory_t *new_pd = (page_directory_t*)pmm_alloc_page();
    if (new_pd == 0) {
        // terminal_write_string("VMM FATAL: Failed to allocate Page Directory!");
        return;
    }

    // Очищаем PDT нулями
    // Очистка нового Каталога Страниц перед использованием
    memset(new_pd, 0, PAGE_SIZE);


    // 2. Устанавливаем текущий каталог страниц
    current_page_directory = new_pd;

    // 3. Создаем Идентичное Отображение (Identity Mapping) для первых 4 МБ
    // Это гарантирует, что код ядра, загруженный GRUB'ом, продолжает работать
    uint32_t addr;
    for (addr = 0; addr < 0x400000; addr += PAGE_SIZE) { // 4MB
        vmm_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }

    // 4. Включаем пейджинг
    vmm_switch_directory(new_pd);
}


// ----------------------------------------------------
// 3. vmm_switch_directory: Загрузка CR3 и Включение
// ----------------------------------------------------
void vmm_switch_directory(page_directory_t *dir) {
    // 1. Загрузить физический адрес PDT в регистр CR3
    uint32_t pd_phys_addr = (uint32_t)dir;

    __asm__ __volatile__ (
        "mov %0, %%cr3" :: "r"(pd_phys_addr)
    );

    // 2. Установить бит PG (Paging Enable) в CR0
    uint32_t cr0;
    __asm__ __volatile__ (
        "mov %%cr0, %0" : "=r"(cr0)
    );
    cr0 |= 0x80000000; // Установка бита PG (Paging Enable)
    __asm__ __volatile__ (
        "mov %0, %%cr0" :: "r"(cr0)
    );
}