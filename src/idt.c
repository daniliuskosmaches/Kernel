// src/idt.c - ПОЛНАЯ ИСПРАВЛЕННАЯ ВЕРСИЯ

#include "idt.h"
#include "isr.h"
#include "vga.h"
#include "string.h"
#include "system.h"

// ============================================================
// ГЛОБАЛЬНЫЕ ОПРЕДЕЛЕНИЯ
// ============================================================

// Массив обработчиков прерываний (256 векторов)
void (*interrupt_handlers[256])(registers_t *regs) = {0};

// Таблица IDT (256 записей)
struct idt_entry idt[256];
struct idt_ptr idtp;

#define KERNEL_CS 0x08  // Селектор сегмента кода ядра

// Порты контроллера прерываний (PIC)
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// ============================================================
// ОБЪЯВЛЕНИЯ ВНЕШНИХ ISR (из isr.asm)
// ============================================================

// Исключения процессора (ISR 0-31)
extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

// Аппаратные прерывания (IRQ 0-15)
extern void irq0(); extern void irq1(); extern void irq2(); extern void irq3();
extern void irq4(); extern void irq5(); extern void irq6(); extern void irq7();
extern void irq8(); extern void irq9(); extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// ============================================================
// УСТАНОВКА ШЛЮЗА В IDT
// ============================================================
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero     = 0;
    idt[num].flags    = flags;  // 0x8E = Present, Ring 0, 32-bit Interrupt Gate
}

// ============================================================
// ПЕРЕНАЗНАЧЕНИЕ PIC (ИСПРАВЛЕНО!)
// ============================================================
void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    // Сохраняем текущие маски прерываний
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // ICW1: Начало инициализации PIC в каскадном режиме
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2: Устанавливаем новые векторы прерываний
    // Master PIC: IRQ 0-7 → 0x20-0x27
    // Slave PIC:  IRQ 8-15 → 0x28-0x2F
    outb(PIC1_DATA, offset1);  // Master offset
    outb(PIC2_DATA, offset2);  // Slave offset

    // ICW3: Настройка каскада Master-Slave
    outb(PIC1_DATA, 0x04);  // Master: Slave на линии IRQ2
    outb(PIC2_DATA, 0x02);  // Slave: каскадная идентификация

    // ICW4: Режим работы 8086/88
    outb(PIC1_DATA, 0x01);  // ИСПРАВЛЕНО: было PIC1_COMMAND
    outb(PIC2_DATA, 0x01);  // ИСПРАВЛЕНО: было PIC2_COMMAND

    // Блокируем все прерывания (они будут включены позже)
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

// ============================================================
// ВКЛЮЧЕНИЕ КОНКРЕТНОГО IRQ
// ============================================================
void pic_enable_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// ============================================================
// ОТКЛЮЧЕНИЕ КОНКРЕТНОГО IRQ
// ============================================================
void pic_disable_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

// ============================================================
// ГЛАВНЫЙ C-ОБРАБОТЧИК ВСЕХ ПРЕРЫВАНИЙ
// ============================================================
void isr_handler_c(struct registers *regs) {
    // 1. Если есть зарегистрированный обработчик, вызываем его
    if (interrupt_handlers[regs->int_no] != 0) {
        void (*handler)(registers_t *regs) = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        // 2. Для необработанных исключений (0-31) выводим ошибку
        if (regs->int_no < 0x20) {
            terminal_write_string("\n=== UNHANDLED EXCEPTION ===\n");
            terminal_write_string("Exception: 0x");
            terminal_write_hex(regs->int_no);
            terminal_write_string("\nEIP: 0x");
            terminal_write_hex(regs->eip);
            terminal_write_string("\nError Code: 0x");
            terminal_write_hex(regs->err_code);
            terminal_write_string("\n");

            // Останавливаем систему
            for(;;) __asm__ volatile("cli; hlt");
        }
    }

    // 3. Отправляем EOI (End of Interrupt) контроллеру прерываний
    // Только для аппаратных прерываний (IRQ 0-15, векторы 0x20-0x2F)
    if (regs->int_no >= 0x20 && regs->int_no <= 0x2F) {
        // Если прерывание от Slave PIC (IRQ 8-15), отправляем EOI обоим
        if (regs->int_no >= 0x28) {
            outb(PIC2_COMMAND, 0x20);  // EOI для Slave PIC
        }
        outb(PIC1_COMMAND, 0x20);  // EOI для Master PIC
    }
}

// ============================================================
// РЕГИСТРАЦИЯ ОБРАБОТЧИКА ПРЕРЫВАНИЯ
// ============================================================
void register_interrupt_handler(uint8_t irq_number, void (*handler)(registers_t *regs)) {
    interrupt_handlers[irq_number] = handler;
}

// ============================================================
// ОБРАБОТЧИК PAGE FAULT (Исключение 14)
// ============================================================
void page_fault_handler_c(registers_t *regs) {
    // Получаем адрес, вызвавший Page Fault, из регистра CR2
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    uint32_t error_code = regs->err_code;

    terminal_write_string("\n");
    terminal_write_string("========================================\n");
    terminal_write_string("         PAGE FAULT DETECTED           \n");
    terminal_write_string("========================================\n");
    terminal_write_string("Faulting Address: 0x");
    terminal_write_hex(faulting_address);
    terminal_write_string("\nInstruction Pointer (EIP): 0x");
    terminal_write_hex(regs->eip);
    terminal_write_string("\nError Code: 0x");
    terminal_write_hex(error_code);
    terminal_write_string("\n\n");

    // Анализ кода ошибки
    terminal_write_string("Error Details:\n");

    if (!(error_code & 0x1)) {
        terminal_write_string("  - Page not present in memory\n");
    } else {
        terminal_write_string("  - Page protection violation\n");
    }

    if (error_code & 0x2) {
        terminal_write_string("  - Write operation\n");
    } else {
        terminal_write_string("  - Read operation\n");
    }

    if (error_code & 0x4) {
        terminal_write_string("  - User mode (Ring 3)\n");
    } else {
        terminal_write_string("  - Kernel mode (Ring 0)\n");
    }

    if (error_code & 0x8) {
        terminal_write_string("  - Reserved bits overwrite\n");
    }

    if (error_code & 0x10) {
        terminal_write_string("  - Instruction fetch\n");
    }

    terminal_write_string("\nKERNEL PANIC: System halted.\n");
    terminal_write_string("========================================\n");

    // Останавливаем систему
    for(;;) __asm__ volatile("cli; hlt");
}

// ============================================================
// МАССИВЫ ВЕКТОРОВ ДЛЯ ЦИКЛИЧЕСКОЙ УСТАНОВКИ
// ============================================================

void (*isr_vectors[32])(void) = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
    isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

void (*irq_vectors[16])(void) = {
    irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
    irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
};

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ УСТАНОВКИ IDT
// ============================================================
void idt_install(void) {
    // 1. Настраиваем указатель IDT
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // 2. Очищаем таблицу IDT
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // 3. Устанавливаем все 32 исключения процессора (ISR 0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (unsigned int)isr_vectors[i], KERNEL_CS, 0x8E);
    }

    // 4. Регистрируем специальные обработчики
    register_interrupt_handler(14, page_fault_handler_c);  // Page Fault

    // 5. Переназначаем PIC (сдвигаем IRQ 0-15 на векторы 0x20-0x2F)
    pic_remap(0x20, 0x28);

    // 6. Устанавливаем все 16 аппаратных прерываний (IRQ 0-15)
    for (int i = 0; i < 16; i++) {
        idt_set_gate(i + 0x20, (unsigned int)irq_vectors[i], KERNEL_CS, 0x8E);
    }

    // 7. Загружаем IDT в процессор
    lidt((void*)&idtp);

    // ВАЖНО: НЕ включаем IRQ здесь!
    // Они будут включены позже в time_install() и init_keyboard()
}
