// src/idt.c
// src/idt.c (ИСПРАВЛЕННЫЙ КОД)

#include "idt.h"
#include "isr.h"
#include "stdint.h"
#include "vga.h"
#include "string.h"
// Убедитесь, что system.h включен (он должен быть включен через idt.h, но на всякий случай)
#include "system.h"

// --- ГЛОБАЛЬНЫЕ ОПРЕДЕЛЕНИЯ ---
// 1. Определение глобального массива обработчиков (выделение памяти)
// ВАЖНО: Это единственное место, где он должен быть определен.
void (*interrupt_handlers[256])(registers_t *regs) = {0};

// 2. Глобальная таблица IDT (256 записей)
struct idt_entry idt[256];
struct idt_ptr idtp;

// Селектор сегмента кода.
#define KERNEL_CS 0x08

// Порты I/O для главного и подчиненного PIC
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// ----------------------------------------------------
// Объявления внешних функций (для избежания 'implicit declaration')
// ----------------------------------------------------
extern void isr_irq0(void);
extern void isr_irq1(void);
extern void terminal_put_char(char c);
extern void terminal_write_string(const char* s);
extern void terminal_write_hex(uint32_t val);
extern void terminal_dump_memory(uint32_t addr, uint32_t size);
extern void timer_handler(registers_t *regs);
extern void page_fault_handler_c(registers_t *regs);

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero     = 0;
    // Флаги: P=1 (присутствует), DPL=0 (Ring 0), S=0 (гейт прерывания), Type=1110b (32-бит)
    idt[num].flags    = flags;
}

// ----------------------------------------------------
// Функция для переназначения PIC
// ----------------------------------------------------
void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    // Сохраняем текущие маски
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // ICW1: Начало инициализации (Cascade mode)
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2: Сдвиг векторов (Critical step!)
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    // ICW3: Соединение PIC (Master - Slave)
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // ICW4: Режим работы (8086 mode)
    outb(PIC1_COMMAND, 0x01); // Исправлено: outb(PIC1_DATA, 0x01); -> outb(PIC1_COMMAND, 0x01);
    outb(PIC2_COMMAND, 0x01);

    // Восстанавливаем сохраненные маски
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

// ----------------------------------------------------
// Главная функция установки IDT
// ----------------------------------------------------
void idt_install(void) {
    // 1. Устанавливаем указатель IDTP (Limit и Base)
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // 2. Устанавливаем обработчики (примеры)
    idt_set_gate(0x20, (unsigned int)isr_irq0, KERNEL_CS, 0x8E); // Таймер
    idt_set_gate(0x21, (unsigned int)isr_irq1, KERNEL_CS, 0x8E); // Клавиатура
    idt_set_gate(14, (unsigned int)isr_irq1, KERNEL_CS, 0x8E); // Page Fault (используем временно isr_irq1, т.к. isr.asm не имеет отдельного обработчика)


    // 3. Переназначаем PIC.
    pic_remap(0x20, 0x28);

    // 4. Загружаем IDT в процессор
    lidt((void*)&idtp);
}

char kbd_us[128] =
{
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t',
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,
    '*',
    0,
  ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',
    0,
    0,
    0,
    0,
};

extern void shell_handle_char(char c);

void keyboard_handler(void) {
    unsigned char scancode = inb(0x60);

    if (scancode < 128) {
        char c = kbd_us[scancode];

        if (c != 0) {
            shell_handle_char(c);
        }
    }
}

// ----------------------------------------------------
// Главный C-обработчик всех прерываний
// ----------------------------------------------------
void isr_handler_c(registers_t *regs) {
    // 1. Диспетчеризация через массив обработчиков
    if (interrupt_handlers[regs->int_no] != 0) {
        interrupt_handlers[regs->int_no](regs);
    }

    // 2. Отправляем EOI на PIC (Только для IRQ прерываний: 0x20 - 0x2F)
    if (regs->int_no >= 0x20 && regs->int_no <= 0x2F) {
        // Проверяем, нужно ли сбросить Slave PIC
        if (regs->int_no >= 0x28) {
            outb(0xA0, 0x20); // Slave PIC EOI
        }
        outb(0x20, 0x20); // Master PIC EOI
    }
    if (regs->int_no >= 0x20 && regs->int_no <= 0x2F) {
        if (regs->int_no >= 0x28) {
            outb(0xA0, 0x20); // Slave PIC EOI
        }
        outb(0x20, 0x20); // Master PIC EOI
    }
}

void page_fault_handler_c(registers_t *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    uint32_t error_code = regs->err_code;

    terminal_write_string("PAGE FAULT! Address: 0x");
    terminal_write_hex(faulting_address);
    terminal_write_string("\n");

    // АНАЛИЗ КОДА ОШИБКИ (Error Code)
    if (!(error_code & 0x1)) {
        terminal_write_string("  Reason: Page Not Present.\n");
    } else {
        terminal_write_string("  Reason: Protection Violation.\n");
    }

    if (error_code & 0x2) {
        terminal_write_string("  Operation: Write.\n");
    } else {
        terminal_write_string("  Operation: Read.\n");
    }

    if (error_code & 0x4) {
        terminal_write_string("  Mode: User (Ring 3).\n");
    } else {
        terminal_write_string("  Mode: Kernel (Ring 0).\n");
    }

    terminal_write_string("KERNEL PANIC: Unrecoverable memory error.\n");
    for(;;); // Остановка
}


void pic_enable_irq(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// ----------------------------------------------------
// Функция для установки обработчиков
// ----------------------------------------------------
void install_interrupt_handler(int irq_number, void (*handler)(registers_t *regs)) {
    // Массив interrupt_handlers определен глобально выше
    interrupt_handlers[irq_number] = handler;
}