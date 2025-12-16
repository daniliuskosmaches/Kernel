//
// Created by nazar on 09.12.25.
//
// src/pmm.c
#include "pmm.h"// Если вы используете заголовочный файл для PMM
#define PAGE_SIZE 4096
#define MAX_PHYS_MEM 0x40000000 // 1 GB (для примера)
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
    // 1. Устанавливаем адрес Битмапа сразу после ядра
    // Мы должны убедиться, что он выровнен по 4К.
    unsigned int end_of_kernel = (unsigned int)&placement_address;

    // Битмап должен быть выровнен на границу страницы
    if (end_of_kernel & 0xFFF) {
        end_of_kernel &= 0xFFFFF000;
        end_of_kernel += 0x1000;
    }

    pmm_bitmap = (unsigned int *)end_of_kernel;

    // 2. Сначала помечаем ВСЮ память как ЗАНЯТУЮ
    // Размер битмапа: (MAX_PAGES / 32) * 4 байта
    unsigned int bitmap_size = MAX_PAGES / 8;

    // Инициализируем битмап нулями (все свободно) - НЕТ! Сначала все занято.
    for (unsigned int i = 0; i < (MAX_PAGES / 32); i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    // 3. Сканируем карту памяти Multiboot и освобождаем доступные блоки
    struct multiboot_mmap_entry *mmap = (struct multiboot_mmap_entry *)mbi->mmap_addr;

    while ((unsigned int)mmap < (mbi->mmap_addr + mbi->mmap_length)) {
        if (mmap->type == 1) { // 1 = Доступная (Available) память

            // Выравниваем начальный адрес на границу страницы
            unsigned int block_start = (unsigned int)mmap->addr;
            unsigned int block_len = (unsigned int)mmap->len;

            // Сбрасываем (освобождаем) все страницы в этом блоке
            for (unsigned int i = 0; i < block_len; i += PAGE_SIZE) {
                // Если адрес находится в пределах MAX_PHYS_MEM, освобождаем
                if ((block_start + i) < MAX_PHYS_MEM) {
                    clear_frame((block_start + i) / PAGE_SIZE);
                    total_pages++;
                }
            }
        }

        // Переход к следующей записи
        mmap = (struct multiboot_mmap_entry *)((unsigned int)mmap + mmap->size + sizeof(mmap->size));
    }

    // 4. Помечаем страницы, занятые ядром и самим Битмапом, как ЗАНЯТЫЕ

    // Конец ядра + размер битмапа
    unsigned int pmm_end = (unsigned int)pmm_bitmap + bitmap_size;

    // Резервируем память от 0 до конца PMM
    for (unsigned int i = 0; i < pmm_end; i += PAGE_SIZE) {
        set_frame(i / PAGE_SIZE);
        used_pages++;
    }

    // Теперь можно использовать pmm_alloc_page()
}

void *pmm_alloc_page(void) {
    if (used_pages >= total_pages) {
        return 0; // Память кончилась! (Null-указатель)
    }

    // Ищем первый свободный бит (0)
    for (unsigned int i = 0; i < total_pages; i++) {
        if (test_frame(i) == 0) {
            // Нашли свободную страницу с индексом 'i'

            // 1. Занимаем страницу в Битмапе
            set_frame(i);
            used_pages++;

            // 2. Возвращаем физический адрес страницы
            // Индекс * Размер_Страницы = Физический Адрес
            return (void *)(i * PAGE_SIZE);
        }
    }
    return 0; // Не найдено (хотя это не должно произойти, если счетчики верны)
}

void pmm_free_page(void *p) {
    // 1. Вычисляем индекс кадра по адресу
    unsigned int frame_index = (unsigned int)p / PAGE_SIZE;

    // 2. Проверяем, что адрес действителен и не выходит за пределы
    if (frame_index >= total_pages) {
        return; // Ошибка: адрес вне диапазона
    }

    // 3. Освобождаем страницу, если она была занята
    if (test_frame(frame_index) == 1) {
        clear_frame(frame_index);
        used_pages--;
    }
}