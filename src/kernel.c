// src/kernel.c (ФИНАЛЬНАЯ ИСПРАВЛЕННАЯ ВЕРСИЯ)

// Главная функция, вызываемая из entry.asm

#include "idt.h"
#include "multiboot.h"
#include "vga.h"
#include "vmm.h"
#include "timer.h"
#include "task.h"
#include "kheap.h"// Для task_init

// Объявления внешних функций, необходимые для kmain
// ----------------------------------------------------------------------
extern void pmm_init(multiboot_info_t *mbi);
extern void vmm_init(void);
extern void shell_init(void);

// КРИТИЧЕСКИ НЕДОСТАЮЩИЕ:
extern void kheap_init(void);    // Инициализация кучи
extern void init_keyboard(void); // Инициализация клавиатуры (в keyboard.c)
extern void task_init(void);     // Инициализация подсистемы задач

// ----------------------------------------------------------------------


// Вспомогательная функция для тестовой задачи (пример)
void task_b_entry() {
    terminal_write_string("Task B running...\\n");
    for (;;) {
        // Если переключение работает, эта строка будет выводиться по очереди
        terminal_write_string("B");
    }
}


void kmain(unsigned int multiboot_magic, multiboot_info_t *mbi) {
    // 1. Инициализация VGA ПЕРВЫМ (чтобы видеть ошибки)
    terminal_initialize();
    terminal_write_string("Booting MyOS...\\n");

    // Проверка Multiboot Magic и наличия карты памяти
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_write_string("ERROR: Invalid multiboot magic!\\n");
        for(;;);
    }
    if (!(mbi->flags & MULTIBOOT_FLAG_MMAP)) {
        terminal_write_string("ERROR: No memory map provided!\\n");
        for(;;);
    }
    terminal_write_string("Phase 1: Terminal Ready.\\n");


    // 2. Установка IDT (Обработчики сбоев и прерываний ВЫКЛЮЧЕНЫ)
    idt_install();
    terminal_write_string("Phase 2: IDT Ready.\\n");


    // 3. Управление Памятью (PMM/VMM)
    terminal_write_string("Initializing PMM and VMM...\\n");
    pmm_init(mbi);
    vmm_init();
    terminal_write_string("Phase 3: Memory Managers Ready.\\n");


    // 4. ИНИЦИАЛИЗАЦИЯ КУЧИ (Kernel Heap) <-- КРИТИЧЕСКИЙ ВЫЗОВ
    // Теперь kmalloc/kfree будут работать, что необходимо для task.c и shell.c
    terminal_write_string("Phase 4: Initializing Kernel Heap...\\n");
    kheap_init();
    terminal_write_string("Kernel Heap Ready.\\n");


    // 5. ИНИЦИАЛИЗАЦИЯ МНОГОЗАДАЧНОСТИ
    terminal_write_string("Phase 5: Initializing Tasking...\\n");
    task_init();

    // Создаем тестовую задачу (необязательно, но полезно для проверки планировщика)
    // create_task(task_b_entry, 0); // Раскомментировать, когда task.c будет готов

    terminal_write_string("Tasking Subsystem Ready.\\n");


    // 6. Установка Таймера (IRQ0) и Клавиатуры (IRQ1)
    terminal_write_string("Phase 6: Installing Timer and Keyboard...\\n");

    // Настраивает PIT и регистрирует обработчик (timer_handler_c)
    time_install(100);
    terminal_write_string("Timer and IRQ0 Ready.\\n");

    // Инициализация Клавиатуры (включает IRQ1) <-- КРИТИЧЕСКИЙ ВЫЗОВ
    init_keyboard();
    terminal_write_string("Keyboard and IRQ1 Ready.\\n");


    // 7. Включаем прерывания
    __asm__ volatile ("sti");
    terminal_write_string("Phase 7: Interrupts Enabled (STI).\\n");


    // 8. Инициализация Shell (теперь он может использовать kmalloc)
    terminal_write_string("Phase 8: Initializing Shell...\\n");
    shell_init();

    terminal_write_string("\\n");
    terminal_write_string("===========================\\n");
    terminal_write_string("Welcome to MyOS!\\n");
    terminal_write_string("Type 'help' for commands.\\n");
    terminal_write_string("===========================\\n");

    // 9. Главный цикл ядра: ждем прерываний (или планировщика)
    for (;;) {
        __asm__ volatile ("hlt"); // Останавливаем ЦП до следующего прерывания
    }
}