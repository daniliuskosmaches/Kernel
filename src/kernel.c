// src/kernel.c

// Главная функция, вызываемая из entry.asm
#include "system.h"
#include "idt.h"
#include "multiboot.h"

extern void pmm_init(multiboot_info_t *mbi);
extern void timer_install(unsigned long freq);

void kmain(unsigned int multiboot_magic, multiboot_info_t *mbi)
{
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {

        return;
    }

    // Проверка, что есть карта памяти
    if (!(mbi->flags & MULTIBOOT_FLAG_MMAP)) {

        return;
    }




    // 2. Настройка Таймера
    timer_install(100); // Устанавливаем частоту 100 Гц

    // 3. Включаем прерывания (КРИТИЧНО)
    __asm__ __volatile__ ("sti");

    // Для начала, просто выведем символ, если это прерывание от PIC
    idt_install();

    asm volatile("sti");
    // Адрес буфера VGA text mode. Ядро пишет прямо сюда.
    unsigned char *vga = (unsigned char *)0xB8000;

    // Выводим "OK" в левый верхний угол (строка 0, столбец 0).
    // Байт 0: Символ 'O'
    vga[0] = 'O';
    // Байт 1: Аттрибут цвета (0x02 = зеленый на черном)
    vga[1] = 0x02;

    // Выводим "K"
    vga[2] = 'K';
    vga[3] = 0x02;


    // Бесконечный цикл: ядро должно работать вечно
    while (1) {
        ;
    }
}

