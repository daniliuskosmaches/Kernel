// src/kernel.c - ИСПРАВЛЕННАЯ ВЕРСИЯ
#include "../include/vga.h"
#include "../include/multiboot.h"
#include "../include/pmm.h"
#include "../include/core/vmm.h"
#include "../include/idt.h"
#include "../include/timer.h"
#include "../include/syscall.h"
#include "../include/keyboard.h"
#include "../include/app/shell.h"
#include "../include/core/kheap.h"
#include "../include/gdt.h"
#include "../include/vfs/vfs.h"
#include "../include/commands.h"

extern uint32_t placement_address;

void kmain(unsigned int multiboot_magic, multiboot_info_t *mbi) {
    // 1. Инициализация экрана
    terminal_initialize();
    terminal_write_string("========================================\n");
    terminal_write_string("       MyOS v1.0 - Booting...          \n");
    terminal_write_string("========================================\n\n");

    // 2. Проверка Multiboot
    terminal_write_string("[1/9] Checking Multiboot... ");
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_write_string("FAILED: Invalid Magic\n");
        while (1) __asm__ volatile("hlt");
    }
    terminal_write_string("OK\n");


    gdt_install();

    // 3. Физическая память
    terminal_write_string("[2/9] Initializing PMM... ");
    pmm_init(mbi);
    terminal_write_string("OK\n");



    // 4. IDT — ОБЯЗАТЕЛЬНО до включения прерываний
    terminal_write_string("[3/9] Installing IDT & Syscalls... ");
    idt_install();
    register_interrupt_handler(128, syscall_handler);
    terminal_write_string("OK\n");



    // 5. Виртуальная память
    terminal_write_string("[4/9] Initializing VMM... ");
    vmm_init();
    terminal_write_string("OK\n");

    __asm__ volatile ("sti");

    // 6. Куча
    terminal_write_string("[5/9] Initializing Kernel Heap... ");
    uint32_t bitmap_size = (0x10000000 / 4096) / 8;
    uint32_t heap_start = (uint32_t)&placement_address + bitmap_size + 0x2000;
    kheap_init(heap_start, heap_start + 0x1000000);
    terminal_write_string("OK\n");

    // 7. Многозадачность
    terminal_write_string("[6/9] Initializing Tasking... ");
    // task_init();
    terminal_write_string("OK\n");

    // 8. Таймер
    terminal_write_string("[7/9] Installing Timer... ");
    // timer_install(100);
    terminal_write_string("OK\n");

    // 9. Клавиатура — регистрируем обработчик и включаем IRQ1 в PIC
    //    НО прерывания ещё выключены (sti ниже), поэтому безопасно
    terminal_write_string("[8/9] Initializing Keyboard... ");
    init_keyboard();
    terminal_write_string("OK\n");

    // FIX: Включаем прерывания ПОСЛЕДНИМ — после всей инициализации
    terminal_write_string("[9/11] Enabling Interrupts... ");
    __asm__ volatile ("sti");
    terminal_write_string("OK\n\n");

    terminal_write_string("[10/11] initiilization vfs");
    vfs_init();
    terminal_write_string("OK\n\n");

    terminal_write_string("[11/11] rmfs initinilization");
    ramfs_init();
    terminal_write_string("OK\n\n");



    terminal_write_string("System Ready. Launching Shell...\n");

    // Запуск шелла
    terminal_clear_screen();
    shell_init();
    terminal_write_string("> ");

    // FIX: Главный цикл — используем keyboard_get_char + shell_handle_char
    // НЕ вызываем keyboard_line_ready здесь — она тоже двигает tail очереди
    while (1) {
        // FIX: hlt ПЕРЕД чтением — ждём прерывания, потом обрабатываем
        // Это важно: прерывание разбудит CPU, мы прочитаем символ
        __asm__ volatile("hlt");

        char c = keyboard_get_char();
        if (c != 0) {
            shell_handle_char(c);
            // FIX: выводим приглашение после Enter
            if (c == '\n') {
                terminal_write_string("> ");
            }
        }
    }
}