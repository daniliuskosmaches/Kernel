// src/kernel.c

// Главная функция, вызываемая из entry.asm
#include "system.h"
#include "idt.h"
#include "idt.c"
void kmain(void)
{

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