// src/pmm.c - ИСПРАВЛЕННАЯ ВЕРСИЯ

#include "pmm.h"
#include "multiboot.h"
#include "vga.h"  // Для отладочных сообщений

#define PAGE_SIZE 4096
#define MAX_PHYS_MEM 0x10000000 // 256 MB (вместо 1GB) - чтобы bitmap влез в 16MB
#define MAX_PAGES (MAX_PHYS_MEM / PAGE_SIZE)

extern unsigned int placement_address; // Получаем адрес из linker.ld

unsigned int *pmm_bitmap;
unsigned int total_pages = 0;
unsigned int used_pages = 0;

void set_frame(unsigned int frame_index) {
    pmm_bitmap[frame_index / 32] |= (1 << (frame_index % 32));
}

void clear_frame(unsigned int frame_index) {
    pmm_bitmap[frame_index / 32] &= ~(1 << (frame_index % 32));
}

int test_frame(unsigned int frame_index) {
    return pmm_bitmap[frame_index / 32] & (1 << (frame_index % 32));
}

void pmm_init(multiboot_info_t *mbi) {



    terminal_write_string("  -> Placement address: 0x");
    terminal_write_hex((unsigned int)&placement_address);
    terminal_write_string("\n");

    // 1. Устанавливаем адрес Битмапа сразу после ядра
    unsigned int end_of_kernel = (unsigned int)&placement_address;

    // Битмап должен быть выровнен на границу страницы
    if (end_of_kernel & 0xFFF) {
        end_of_kernel &= 0xFFFFF000;
        end_of_kernel += 0x1000;
    }

    pmm_bitmap = (unsigned int *)end_of_kernel;

    terminal_write_string("  -> PMM Bitmap at: 0x");
    terminal_write_hex((unsigned int)pmm_bitmap);
    terminal_write_string("\n");

    // 2. Размер битмапа
    unsigned int bitmap_size = MAX_PAGES / 8;
    terminal_write_string("  -> Bitmap size: 0x");
    terminal_write_hex(bitmap_size);
    terminal_write_string(" bytes\n");

    unsigned int bitmap_end = (unsigned int)pmm_bitmap + bitmap_size;
    terminal_write_string("  -> Bitmap ends at: 0x");
    terminal_write_hex(bitmap_end);
    terminal_write_string("\n");

    // КРИТИЧЕСКАЯ ПРОВЕРКА: Убедимся, что bitmap влезает в первые 16MB
    // (которые будут identity mapped в VMM)
    if (bitmap_end > 0x1000000) {
        terminal_write_string("WARNING: Bitmap extends beyond 16MB!\n");
        terminal_write_string("Consider reducing MAX_PHYS_MEM or increasing identity mapping\n");
    }

    // 3. Сначала помечаем ВСЮ память как ЗАНЯТУЮ
    terminal_write_string("  -> Initializing bitmap... ");
    for (unsigned int i = 0; i < (MAX_PAGES / 32); i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }
    terminal_write_string("OK\n");

    // 4. Сканируем карту памяти Multiboot и освобождаем доступные блоки
    terminal_write_string("  -> Scanning Multiboot memory map:\n");
    terminal_write_string("     mmap_addr: 0x");
    terminal_write_hex(mbi->mmap_addr);
    terminal_write_string(", mmap_length: 0x");
    terminal_write_hex(mbi->mmap_length);
    terminal_write_string("\n");

    multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
    multiboot_memory_map_t *mmap_end = (multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);

    unsigned int available_blocks = 0;
    while (mmap < mmap_end) {
        terminal_write_string("     Entry: addr=0x");
        terminal_write_hex((unsigned int)mmap->addr);
        terminal_write_string(", len=0x");
        terminal_write_hex((unsigned int)mmap->len);
        terminal_write_string(", type=");
        terminal_write_hex(mmap->type);
        terminal_write_string("\n");

        if (mmap->type == 1) { // 1 = Доступная (Available) память
            available_blocks++;

            terminal_write_string("       -> Available block #");
            terminal_write_hex(available_blocks);
            terminal_write_string("\n");

            // Работаем с 32-битными адресами
            unsigned int block_start = (unsigned int)mmap->addr;
            unsigned int block_len = (unsigned int)mmap->len;
            unsigned int block_end = block_start + block_len;

            // Выравниваем начало вверх на границу страницы
            if (block_start & 0xFFF) {
                block_start = (block_start & 0xFFFFF000) + 0x1000;
            }

            // Выравниваем конец вниз
            block_end = block_end & 0xFFFFF000;

            // Освобождаем страницы в этом блоке
            for (unsigned int addr = block_start; addr < block_end && addr < MAX_PHYS_MEM; addr += PAGE_SIZE) {
                unsigned int frame_idx = addr / PAGE_SIZE;
                clear_frame(frame_idx);
                total_pages++;
            }
        }

        // Переход к следующей записи
        mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(mmap->size));
    }

    terminal_write_string("  -> Total available pages: ");
    terminal_write_hex(total_pages);
    terminal_write_string(" (");
    terminal_write_hex(total_pages * PAGE_SIZE / 1024);
    terminal_write_string(" KB)\n");

    // 5. Помечаем страницы, занятые ядром и самим Битмапом, как ЗАНЯТЫЕ
    terminal_write_string("  -> Reserving kernel and bitmap:\n");

    // ИСПРАВЛЕНИЕ: Резервируем ТОЛЬКО реально используемую память
    // от 0 до конца bitmap (не весь bitmap_end!)
    unsigned int pmm_end = bitmap_end;

    // Выравниваем вверх на границу страницы
    if (pmm_end & 0xFFF) {
        pmm_end = (pmm_end & 0xFFFFF000) + 0x1000;
    }

    terminal_write_string("     Reserving 0x0 - 0x");
    terminal_write_hex(pmm_end);
    terminal_write_string("\n");

    // Резервируем память от 0 до конца PMM
    unsigned int reserved_pages = 0;
    for (unsigned int addr = 0; addr < pmm_end; addr += PAGE_SIZE) {
        unsigned int frame_idx = addr / PAGE_SIZE;

        // Проверяем, была ли эта страница помечена как доступная
        if (test_frame(frame_idx) == 0) {
            // Была свободной - теперь резервируем
            set_frame(frame_idx);
            reserved_pages++;
            // НЕ увеличиваем used_pages здесь! Она не была учтена в total_pages
        } else {
            // Уже была занята (не входила в доступные блоки)
            reserved_pages++;
        }
    }

    // Теперь правильно устанавливаем used_pages
    used_pages = reserved_pages;

    terminal_write_string("     Reserved ");
    terminal_write_hex(reserved_pages);
    terminal_write_string(" pages (");
    terminal_write_hex(reserved_pages * PAGE_SIZE / 1024);
    terminal_write_string(" KB)\n");

    terminal_write_string("  -> Free pages: ");
    terminal_write_hex(total_pages - used_pages);
    terminal_write_string(" (");
    terminal_write_hex((total_pages - used_pages) * PAGE_SIZE / 1024);
    terminal_write_string(" KB)\n");

    // КРИТИЧЕСКАЯ ПРОВЕРКА
    if (total_pages <= used_pages) {
        terminal_write_string("CRITICAL: No free pages available!\n");
        terminal_write_string("Total: ");
        terminal_write_hex(total_pages);
        terminal_write_string(", Used: ");
        terminal_write_hex(used_pages);
        terminal_write_string("\n");
        for(;;) __asm__ volatile("cli; hlt");
    }
}

void *pmm_alloc_page(void) {
    if (used_pages >= total_pages) {
        // Память кончилась!
        terminal_write_string("PMM: Out of memory! (");
        terminal_write_hex(used_pages);
        terminal_write_string("/");
        terminal_write_hex(total_pages);
        terminal_write_string(")\n");
        return 0;
    }

    // Ищем первый свободный бит (0)
    for (unsigned int i = 0; i < MAX_PAGES; i++) {
        if (test_frame(i) == 0) {
            // Нашли свободную страницу с индексом 'i'

            // 1. Занимаем страницу в Битмапе
            set_frame(i);
            used_pages++;

            // 2. Возвращаем физический адрес страницы
            return (void *)(i * PAGE_SIZE);
        }
    }

    // Не найдено (это не должно произойти, если счетчики верны)
    terminal_write_string("PMM: No free frame found (inconsistent state)\n");
    return 0;
}

void pmm_free_page(void *p) {
    // 1. Вычисляем индекс кадра по адресу
    unsigned int frame_index = (unsigned int)p / PAGE_SIZE;

    // 2. Проверяем, что адрес действителен и не выходит за пределы
    if (frame_index >= MAX_PAGES) {
        terminal_write_string("PMM: Attempted to free invalid address 0x");
        terminal_write_hex((unsigned int)p);
        terminal_write_string("\n");
        return;
    }

    // 3. Освобождаем страницу, если она была занята
    if (test_frame(frame_index) == 1) {
        clear_frame(frame_index);
        used_pages--;
    }
}