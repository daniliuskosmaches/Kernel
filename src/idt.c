// src/idt.c
// src/idt.c (ИСПРАВЛЕННЫЙ КОД)

#include "idt.h"
#include "isr.h"
#include "keyboard.h"
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

// src/idt.c (ИСПРАВЛЕННЫЙ idt_install)

// Объявления внешних функций (для всех 32 ISR)
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// Объявления внешних функций (для всех IRQ)
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


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
    outb(PIC1_COMMAND, 0x01);
    outb(PIC2_COMMAND, 0x02);

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



// ----------------------------------------------------
// Главный C-обработчик всех прерываний
// ----------------------------------------------------
// src/idt.c (Фрагмент: isr_handler_c)
// ----------------------------------------------------
// Главный C-обработчик всех прерываний
// ----------------------------------------------------
void isr_handler_c(struct registers *regs) {
    // 1. Диспетчеризация через массив обработчиков
    // Используем массив, заполненный через install_interrupt_handler
    if (interrupt_handlers[regs->int_no] != 0) {
        interrupt_handlers[regs->int_no](regs);
    } else {
        // Для неназначенных исключений (0-19), кроме 14 (Page Fault)
        if (regs->int_no < 0x20 && regs->int_no != 14) {
            terminal_write_string("UNHANDLED EXCEPTION: 0x");
            terminal_write_hex(regs->int_no);
            terminal_write_string("\n");
            // Если вы не хотите, чтобы ядро зависало здесь, добавьте for(;;);
        }
    }

    // 2. Отправляем EOI на PIC (Только для IRQ прерываний: 0x20 - 0x2F)
    if (regs->int_no >= 0x20 && regs->int_no <= 0x2F) {
        // Проверяем, нужно ли сбросить Slave PIC
        if (regs->int_no >= 0x28) {
            outb(0xA0, 0x20); // Slave PIC EOI
        }
        outb(0x20, 0x20); // Master PIC EOI
    }
}

void register_interrupt_handler(uint8_t irq_number, void (*handler)(registers_t *regs)) {
    // Массив interrupt_handlers определен глобально выше
    interrupt_handlers[irq_number] = handler;
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



// Массив указателей на все ISR, чтобы можно было использовать цикл
void (*isr_vectors[32])(void) = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

// Массив указателей на все IRQ
void (*irq_vectors[16])(void) = {
    irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
};


void idt_install(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // 1. Устанавливаем все 32 исключения процессора (ISR 0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (unsigned int)isr_vectors[i], KERNEL_CS, 0x8E);
    }

    // 2. Переназначаем PIC.
    pic_remap(0x20, 0x28);

    // 3. Устанавливаем все 16 прерываний PIC (IRQ 0-15)
    for (int i = 0; i < 16; i++) {
        idt_set_gate(i + 0x20, (unsigned int)irq_vectors[i], KERNEL_CS, 0x8E);
    }

    // 4. Загружаем IDT в процессор
    lidt((void*)&idtp);
} 
pic_enable_irq(0);
pic_enable_irq(1); 