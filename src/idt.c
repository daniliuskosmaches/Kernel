// src/idt.c

#include "idt.h"


// Глобальная таблица IDT (256 записей)

struct idt_ptr idtp;

// Селектор сегмента кода.
// В Multiboot это обычно 0x08, но нужно проверить ваш linker.ld.
#define KERNEL_CS 0x08

// Порты I/O для главного и подчиненного PIC
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// ----------------------------------------------------
// 1. Функция для заполнения одного дескриптора IDT
// ----------------------------------------------------
extern void isr_irq0(void);
extern void isr_irq1(void);
extern void terminal_put_char(char c);
extern void timer_handler(struct registers *regs);
// -----------------------------------------------------------------

// Глобальная таблица IDT (256 записей)
struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero     = 0;
    // Флаги: P=1 (присутствует), DPL=0 (Ring 0), S=0 (гейт прерывания), Type=1110b (32-бит)
    idt[num].flags    = flags;
}

// ----------------------------------------------------
// 2. Функция для переназначения PIC
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
    // Master PIC (IRQ 0-7) начинается с вектора 0x20 (32)
    outb(PIC1_DATA, offset1);
    // Slave PIC (IRQ 8-15) начинается с вектора 0x28 (40)
    outb(PIC2_DATA, offset2);

    // ICW3: Соединение PIC (Master - Slave)
    outb(PIC1_DATA, 0x04); // Slave соединен с IRQ 2 (0000 0100b)
    outb(PIC2_DATA, 0x02); // ID Slave'а (0000 0010b)

    // ICW4: Режим работы (8086 mode)
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Восстанавливаем сохраненные маски
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

// ----------------------------------------------------
// 3. Главная функция установки IDT
// ----------------------------------------------------
void idt_install(void) {
    // 1. Устанавливаем указатель IDTP (Limit и Base)
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // (Необязательный, но рекомендуемый шаг: очистка всей IDT)
    // memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // 2. Устанавливаем обработчики (теперь адреса IDT и IDTP установлены)
    // Установка обработчика для Таймера (IRQ 0 -> 0x20)
    idt_set_gate(0x20, (unsigned int)isr_irq0, KERNEL_CS, 0x8E);

    // Установка обработчика для Клавиатуры (IRQ 1 -> 0x21)
    idt_set_gate(0x21, (unsigned int)isr_irq1, KERNEL_CS, 0x8E);

    // 3. Переназначаем PIC.
    // Это должно быть сделано ДО включения прерываний,
    // чтобы процессор не получил старые, опасные прерывания.
    pic_remap(0x20, 0x28);

    // 4. Загружаем IDT в процессор
    // После этого процессор начнет использовать IDT при прерываниях.
    lidt((void*)&idtp);

    // (ВАЖНО: Активация прерываний "sti" должна быть в kmain, а не здесь.)
}

char kbd_us[128] =
{
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', /* 0x0F - Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* 0x1C - Enter */
    0, /* 0x1D - Left Ctrl */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* 0x2A - Left Shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* 0x36 - Right Shift */
    '*',
    0, /* 0x38 - Left Alt */
  ' ', /* 0x39 - Spacebar */
    0, /* 0x3A - Caps Lock */
    0, /* 0x3B - F1 */
    0, /* 0x3C - F2 */
    0,
    0,
    0,
    0,
    0,
    0, /* 0x43 - F9 */
    0, /* 0x44 - F10 */
    0, /* 0x45 - Num Lock */
    0, /* 0x46 - Scroll Lock */
    '7', /* 0x47 - Keypad 7 */
    '8', /* 0x47 - Keypad 8 */
    '9', /* 0x47 - Keypad 9 */
    '-', /* 0x4A - Keypad - */
    '4', /* 0x4B - Keypad 4 */
    '5', /* 0x4C - Keypad 5 */
    '6', /* 0x4D - Keypad 6 */
    '+', /* 0x4E - Keypad + */
    '1', /* 0x4F - Keypad 1 */
    '2', /* 0x50 - Keypad 2 */
    '3', /* 0x51 - Keypad 3 */
    '0', /* 0x52 - Keypad 0 */
    '.', /* 0x53 - Keypad . */
    0,
    0,
    0, /* 0x57 - F11 */
    0, /* 0x58 - F12 */
};

void keyboard_handler(void) {
    // 1. Считываем скан-код из порта данных клавиатуры (0x60)
    unsigned char scancode = inb(0x60);

    // 2. Проверяем, была ли клавиша НАЖАТА (Scancode < 0x80)
    if (scancode < 128) {
        // 3. Преобразуем скан-код в ASCII
        char c = kbd_us[scancode];

        // 4. Выводим символ, если он действителен (не 0)
        if (c != 0) {
            terminal_put_char(c);
        }
    }
}

void isr_handler_c(struct registers *regs) {
    if (regs->int_no == 0x20) {
        timer_handler(regs);
    }
    if (regs->int_no == 0x21) {
        keyboard_handler();
    }

    // 2. Отправляем EOI на PIC
    if (regs->int_no >= 0x20 && regs->int_no <= 0x2F) {
        if (regs->int_no >= 0x28) {
            outb(0xA0, 0x20); // Slave PIC
        }
        outb(0x20, 0x20); // Master PIC
    }
    // Внимание: Вам нужно будет определить структуру 'registers' позже!

    // Для начала, просто выведем символ, если это прерывание от PIC
    if (regs->int_no >= 0x20 && regs->int_no <= 0x2F) {
        // Здесь будет наш обработчик клавиатуры/таймера

        // Отправка EOI (End of Interrupt) на PIC (ОЧЕНЬ ВАЖНО!)
        // Без этого PIC не будет генерировать новые прерывания.
        if (regs->int_no >= 0x28) {
            outb(0xA0, 0x20); // Slave PIC
        }
        outb(0x20, 0x20); // Master PIC
    }
}

