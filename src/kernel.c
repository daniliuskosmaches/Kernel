// src/kernel.c - ФИНАЛЬНАЯ РАБОЧАЯ ВЕРСИЯ

#include "idt.h"
#include "multiboot.h"
#include "vga.h"
#include "vmm.h"
#include "timer.h"
#include "task.h"
#include "kheap.h"
#include "pmm.h"
#include "shell.h"

// Объявления внешних функций
extern void pmm_init(multiboot_info_t *mbi);
extern void vmm_init(void);
extern void shell_init(void);
extern void kheap_init(void);
extern void init_keyboard(void);
extern void task_init(void);

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ ЯДРА
// ============================================================
void kmain(unsigned int multiboot_magic, multiboot_info_t *mbi) {

    // ========================================================
    // ФАЗА 1: ИНИЦИАЛИЗАЦИЯ ТЕРМИНАЛА
    // ========================================================
    terminal_initialize();
    terminal_write_string("========================================\n");
    terminal_write_string("       MyOS v1.0 - Booting...          \n");
    terminal_write_string("========================================\n\n");

    // ========================================================
    // ФАЗА 2: ПРОВЕРКА MULTIBOOT
    // ========================================================
    terminal_write_string("[1/9] Checking Multiboot... ");

    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_write_string("FAILED\n");
        terminal_write_string("ERROR: Invalid multiboot magic number!\n");
        terminal_write_string("Expected: 0x");
        terminal_write_hex(MULTIBOOT_BOOTLOADER_MAGIC);
        terminal_write_string("\nGot: 0x");
        terminal_write_hex(multiboot_magic);
        terminal_write_string("\n");
        for(;;) __asm__ volatile("cli; hlt");
    }

    if (!(mbi->flags & MULTIBOOT_FLAG_MMAP)) {
        terminal_write_string("FAILED\n");
        terminal_write_string("ERROR: No memory map provided by bootloader!\n");
        for(;;) __asm__ volatile("cli; hlt");
    }

    terminal_write_string("OK\n");

    // ========================================================
    // ФАЗА 3: ИНИЦИАЛИЗАЦИЯ IDT
    // ========================================================
    terminal_write_string("[2/9] Installing IDT... ");
    idt_install();
    terminal_write_string("OK\n");

    // ========================================================
    // ФАЗА 4: ИНИЦИАЛИЗАЦИЯ УПРАВЛЕНИЯ ПАМЯТЬЮ
    // ========================================================
    terminal_write_string("[3/9] Initializing PMM... ");
    pmm_init(mbi);
    terminal_write_string("OK\n");

    terminal_write_string("[4/9] Initializing VMM... ");
    vmm_init();
    terminal_write_string("OK\n");

    // ========================================================
    // ФАЗА 5: ИНИЦИАЛИЗАЦИЯ КУЧИ
    // ========================================================
    terminal_write_string("[5/9] Initializing Kernel Heap... ");
    kheap_init();

    // Проверяем, работает ли kmalloc
    void* test_ptr = kmalloc(32);
    if (test_ptr == 0) {
        terminal_write_string("FAILED\n");
        terminal_write_string("ERROR: kmalloc() returned NULL!\n");
        for(;;) __asm__ volatile("cli; hlt");
    }
    kfree(test_ptr);
    terminal_write_string("OK\n");

    // ========================================================
    // ФАЗА 6: ИНИЦИАЛИЗАЦИЯ МНОГОЗАДАЧНОСТИ
    // ========================================================
    terminal_write_string("[6/9] Initializing Tasking... ");
    task_init();
    terminal_write_string("OK\n");

    // ========================================================
    // ФАЗА 7: ИНИЦИАЛИЗАЦИЯ ТАЙМЕРА (IRQ0)
    // ========================================================
    terminal_write_string("[7/9] Installing Timer (100 Hz)... ");
    time_install(100);
    terminal_write_string("OK\n");

    // ========================================================
    // ФАЗА 8: ИНИЦИАЛИЗАЦИЯ КЛАВИАТУРЫ (IRQ1)

    // ========================================================
    terminal_write_string("[8/9] Initializing idt... ");
    idt_install();
    terminal_write_string("OK\n");





    // ========================================================
    // ФАЗА 9: ВКЛЮЧЕНИЕ ПРЕРЫВАНИЙ
    // ========================================================
    terminal_write_string("[9/9] Enabling Interrupts... ");
    __asm__ volatile ("sti");
    terminal_write_string("OK\n\n");
    terminal_write_string("initializing the keyboard... ");
    init_keyboard();
    terminal_write_string("OK\n\n");


    terminal_write_string("========================================\n");
    terminal_write_string("        System Initialization          \n");
    terminal_write_string("              Complete!                \n");
    terminal_write_string("========================================\n\n");

    // ========================================================
    // ФИНАЛЬНОЕ СООБЩЕНИЕ
    // ========================================================
    terminal_write_string("========================================\n");
    terminal_write_string("    Boot Complete! System Ready.       \n");
    terminal_write_string("========================================\n\n");

    // Небольшая задержка для красоты
    for (volatile int i = 0; i < 10000000; i++);

    // ========================================================
    // ЗАПУСК КОМАНДНОЙ ОБОЛОЧКИ
    // ========================================================
    terminal_clear_screen();
    shell_init();

    // ========================================================
    // ГЛАВНЫЙ ЦИКЛ ЯДРА
    // ========================================================
    // Ядро просто ждет прерываний (таймер, клавиатура)

}