// src/kernel.c

// Главная функция, вызываемая из entry.asm
#include "system.h"
#include "idt.h"
#include "multiboot.h"
#include "task.h"
#include "vga.h"
#include "vmm.h"

extern void pmm_init(multiboot_info_t *mbi);
extern void timer_install(unsigned long freq);
void task_b_entry() {
    // Эта функция будет выполняться второй задачей
    terminal_write_string("Task B running...\n");
    for (;;) {
        // Если переключение работает, эта строка будет выводиться по очереди с основной задачей
        terminal_write_string("B");
    }
}


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
    terminal_initialize();

    vmm_init();

    task_init();

    task_t *task_b = fork();

    // 2. Устанавливаем EIP новой задачи на функцию task_b_entry
    task_b->eip = (uint32_t)task_b_entry;

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

    terminal_write_string("Task A (Kernel) running...\n");
    for (;;) {
        // Task A будет выводить 'A', а Task B — 'B'.
        // При переключении контекста (по таймеру) они будут выводить по очереди.
        terminal_write_string("A");
    }
    while (1) {

    }




}






