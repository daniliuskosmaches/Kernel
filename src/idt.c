// src/idt.c - ВИПРАВЛЕНА ВЕРСІЯ (БЕЗ TRIPLE FAULT)
#include "idt.h"
#include "isr.h"
#include "vga.h"
#include "string.h"
#include "system.h"

// Масив обробників переривань
void (*interrupt_handlers[256])(registers_t *regs) = {0};

// Таблиця IDT
struct idt_entry idt[256];
struct idt_ptr idtp;

#define KERNEL_CS 0x08
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// Зовнішні ISR
extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();
extern void isr128();

extern void irq0(); extern void irq1(); extern void irq2(); extern void irq3();
extern void irq4(); extern void irq5(); extern void irq6(); extern void irq7();
extern void irq8(); extern void irq9(); extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// ===== ДОДАЄМО IDT_LOAD =====


void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero     = 0;
    idt[num].flags    = flags;
}

void isr_handler_c(registers_t *regs) {
    // Викликаємо обробник, якщо зареєстрований
    if (interrupt_handlers[regs->int_no] != 0) {
        void (*handler)(registers_t *r) = interrupt_handlers[regs->int_no];
        handler(regs);
    }

    // EOI для PIC
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);  // Slave PIC
    }
    outb(0x20, 0x20);      // Master PIC
}

void register_interrupt_handler(uint8_t n, void (*handler)(registers_t *)) {
    interrupt_handlers[n] = handler;
}

void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    // Зберігаємо поточні маски
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // ICW1: Початок ініціалізації
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2: Вектори переривань
    outb(PIC1_DATA, offset1);  // Master: IRQ 0-7 → 0x20-0x27
    outb(PIC2_DATA, offset2);  // Slave:  IRQ 8-15 → 0x28-0x2F

    // ICW3: Налаштування каскаду
    outb(PIC1_DATA, 0x04);  // Master: Slave на IRQ2
    outb(PIC2_DATA, 0x02);  // Slave: каскад

    // ICW4: Режим 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // КРИТИЧНО: Блокуємо ВСІ переривання спочатку!
    // Вони будуть розблоковані пізніше вручну
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

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

void page_fault_handler_c(registers_t *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    terminal_write_string("\n=== PAGE FAULT ===\n");
    terminal_write_string("Address: ");
    terminal_write_hex(faulting_address);
    terminal_write_string("\nEIP: ");
    terminal_write_hex(regs->eip);
    terminal_write_string("\nError: ");
    terminal_write_hex(regs->err_code);
    terminal_write_string("\nSystem halted.\n");

    for(;;) __asm__ volatile("cli; hlt");
}

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

void idt_install(void) {
    // 1. Налаштовуємо IDTP
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // 2. Очищаємо IDT
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // 3. Встановлюємо ISR 0-31 (виключення процесора)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (unsigned int)isr_vectors[i], KERNEL_CS, 0x8E);
    }

    // 4. Реєструємо обробник Page Fault
    register_interrupt_handler(14, page_fault_handler_c);

    // 5. Переназначаємо PIC (IRQ → 0x20-0x2F)
    pic_remap(0x20, 0x28);

    // 6. Встановлюємо IRQ 0-15 (апаратні переривання)
    for (int i = 0; i < 16; i++) {
        idt_set_gate(i + 0x20, (unsigned int)irq_vectors[i], KERNEL_CS, 0x8E);
    }

    // 7. Системний виклик INT 0x80
    idt_set_gate(128, (unsigned int)isr128, KERNEL_CS, 0xEE);

    // 8. Завантажуємо IDT в процесор
    idt_load();

    // ВАЖЛИВО: НЕ вмикаємо переривання тут!
    // Вони будуть увімкнені після ініціалізації всіх пристроїв
    terminal_write_string("IDT installed (interrupts still disabled)\n");
}