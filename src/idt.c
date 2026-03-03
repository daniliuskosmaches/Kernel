// src/idt.c - ИСПРАВЛЕННАЯ ВЕРСИЯ
#include "../include/idt.h"
#include "../include/isr.h"
#include "../include/vga.h"
#include "../include/lib/string.h"
#include "../include/system.h"

void (*interrupt_handlers[256])(registers_t *regs) = {0};

struct idt_entry idt[256];
struct idt_ptr idtp;

#define KERNEL_CS 0x08
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();
extern void isr128();

extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// Названия исключений для отладки
static const char *exception_messages[] = {
    "Division By Zero",          // 0
    "Debug",                     // 1
    "Non Maskable Interrupt",    // 2
    "Breakpoint",                // 3
    "Into Detected Overflow",    // 4
    "Out of Bounds",             // 5
    "Invalid Opcode",            // 6
    "No Coprocessor",            // 7
    "Double Fault",              // 8
    "Coprocessor Segment Overrun", // 9
    "Bad TSS",                   // 10
    "Segment Not Present",       // 11
    "Stack Fault",               // 12
    "General Protection Fault",  // 13
    "Page Fault",                // 14
    "Unknown Interrupt",         // 15
    "Coprocessor Fault",         // 16
    "Alignment Check",           // 17
    "Machine Check",             // 18
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved"
};

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = sel;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

// FIX: Единый обработчик для всех прерываний с отладочным выводом исключений
void isr_handler_c(registers_t *regs) {
    // Аппаратные прерывания (IRQ)
    if (regs->int_no >= 32 && regs->int_no < 48) {
        // Сначала вызываем обработчик
        if (interrupt_handlers[regs->int_no] != 0) {
            interrupt_handlers[regs->int_no](regs);
        }

        // EOI: сначала slave, потом master
        if (regs->int_no >= 40) {
            outb(PIC2_COMMAND, 0x20);
        }
        outb(PIC1_COMMAND, 0x20);
        return;
    }

    // Системный вызов
    if (regs->int_no == 128) {
        if (interrupt_handlers[128] != 0) {
            interrupt_handlers[128](regs);
        }
        return;
    }

    // FIX: Исключения CPU (0-31) — выводим информацию и останавливаемся
    // Это позволяет увидеть что происходит вместо тихого triple fault
    if (regs->int_no < 32) {
        if (interrupt_handlers[regs->int_no] != 0) {
            // Есть зарегистрированный обработчик (например page_fault_handler_c)
            interrupt_handlers[regs->int_no](regs);
        } else {
            // FIX: Выводим отладочную информацию для любого необработанного исключения
            terminal_write_string("\n\n=== KERNEL EXCEPTION ===\n");
            terminal_write_string("Exception: ");
            if (regs->int_no < 32) {
                terminal_write_string(exception_messages[regs->int_no]);
            }
            terminal_write_string("\nINT#: ");
            terminal_write_hex(regs->int_no);
            terminal_write_string("\nErr:  ");
            terminal_write_hex(regs->err_code);
            terminal_write_string("\nEIP:  ");
            terminal_write_hex(regs->eip);
            terminal_write_string("\nCS:   ");
            terminal_write_hex(regs->cs);
            terminal_write_string("\nEFLAGS: ");
            terminal_write_hex(regs->eflags);
            terminal_write_string("\nSystem halted.\n");
            for (;;) __asm__ volatile("cli; hlt");
        }
    }
}

void register_interrupt_handler(uint8_t n, void (*handler)(registers_t *)) {
    interrupt_handlers[n] = handler;
}

void pic_remap(int offset1, int offset2) {
    // FIX: Сохраняем маски перед реинициализацией
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // ICW1: Начало инициализации
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    // ICW2: Векторы прерываний
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    // ICW3: Каскад
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    // ICW4: Режим 8086
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    // Восстанавливаем маски (все IRQ замаскированы до явного включения)
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    (void)mask1; (void)mask2; // подавляем warning
}

// FIX: io_wait для задержки после outb к PIC
void io_wait(void) {
    outb(0x80, 0);
}

void pic_enable_irq(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void pic_disable_irq(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}

void page_fault_handler_c(registers_t *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    terminal_write_string("\n=== PAGE FAULT ===\n");
    terminal_write_string("Address: "); terminal_write_hex(faulting_address);
    terminal_write_string("\nEIP:     "); terminal_write_hex(regs->eip);
    terminal_write_string("\nError:   "); terminal_write_hex(regs->err_code);
    terminal_write_string("\n  Present:    "); terminal_write_hex(regs->err_code & 0x1);
    terminal_write_string("\n  Write:      "); terminal_write_hex((regs->err_code >> 1) & 0x1);
    terminal_write_string("\n  User:       "); terminal_write_hex((regs->err_code >> 2) & 0x1);
    terminal_write_string("\nSystem halted.\n");
    for (;;) __asm__ volatile("cli; hlt");
}

void (*isr_vectors[32])(void) = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

void (*irq_vectors[16])(void) = {
    irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7,
    irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15
};

void idt_install(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (unsigned int)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // ISR 0-31: исключения CPU
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (unsigned int)isr_vectors[i], KERNEL_CS, 0x8E);
    }

    // Page fault обработчик
    register_interrupt_handler(14, page_fault_handler_c);

    // Ремап PIC
    pic_remap(0x20, 0x28);

    // IRQ 0-15 → INT 32-47
    for (int i = 0; i < 16; i++) {
        idt_set_gate(i + 0x20, (unsigned int)irq_vectors[i], KERNEL_CS, 0x8E);
    }

    // Системный вызов INT 0x80
    idt_set_gate(128, (unsigned int)isr128, KERNEL_CS, 0xEE);

    // Загружаем IDT
    idt_load();

    terminal_write_string("IDT installed (interrupts still disabled)\n");
}