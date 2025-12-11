// src/kernel.c

// Главная функция, вызываемая из entry.asm
// src/kernel.c

// Главная функция, вызываемая из entry.asm
#include "system.h"
#include "idt.h"
#include "multiboot.h"
#include "isr.h"
#include "vga.h"
#include "vmm.h"
#include "timer.h"


// Объявления внешних функций для PMM и Таймера
extern void pmm_init(multiboot_info_t *mbi);
extern void time_install(unsigned long freq);
extern void timer_handler(registers_t *regs);
void task_b_entry() {
    terminal_write_string("Task B running...\n");
    for (;;) {
        // Если переключение работает, эта строка будет выводиться по очереди с основной задачей
        terminal_write_string("B");
    }
}


void kmain(unsigned int multiboot_magic, multiboot_info_t *mbi) {
    // 1. Инициализация VGA ПЕРВЫМ (чтобы видеть ошибки)
    terminal_initialize();
    terminal_write_string("Booting MyOS...\n");

    // Проверка Multiboot Magic и наличия карты памяти
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_write_string("ERROR: Invalid multiboot magic!\n");
        for(;;);
    }
    if (!(mbi->flags & MULTIBOOT_FLAG_MMAP)) {
        terminal_write_string("ERROR: No memory map!\n");
        for(;;);
    }

    terminal_write_string("Phase 1: Terminal Ready.\n");

    // 2. Установка IDT (Обработчики сбоев)
    idt_install();
    terminal_write_string("Phase 2: IDT Ready.\n");

    // 3. Управление Памятью (КРИТИЧЕСКИ ВАЖНЫЙ БЛОК)

    // 3A. Инициализация Физической Памяти (pmm)
    terminal_write_string("Initializing PMM...\n");
    pmm_init(mbi);
    terminal_write_string("Phase 3A: PMM Ready.\n");

    // 3B. Инициализация Виртуальной Памяти (vmm)
    terminal_write_string("Initializing VMM...\n");
    vmm_init();
    terminal_write_string("Phase 3B: VMM Ready.\n");

    // 4. Инициализация Shell
    terminal_write_string("Phase 4: Initializing Shell...\n");
    extern void shell_init(void);
    shell_init();

    // Включаем прерывания для клавиатуры
    __asm__ volatile ("sti");

    terminal_write_string("\n");
    terminal_write_string("===========================\n");
    terminal_write_string("Welcome to MyOS!\n");
    terminal_write_string("Type 'help' for commands\n");
    terminal_write_string("===========================\n\n");

    // Показываем prompt
    extern void shell_print_prompt(void);
    shell_print_prompt();
    install_interrupt_handler(IRQ0, timer_handler);

    // Главный цикл ядра - ждем прерываний
    for (;;) {
        __asm__ volatile ("hlt"); // Ждем прерывания (клавиатура)
    }
}