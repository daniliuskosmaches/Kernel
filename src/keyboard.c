//
// Created by nazar on 11.12.25.
//

// src/keyboard.c

#include "keyboard.h"
#include "vga.h"
#include "system.h" // Для outb, inb, pic_send_eoi
#include "stdint.h"
#include "idt.h"
#include "isr.h"


#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
// Функция для отправки EOI, чтобы PIC знал, что прерывание обработано
extern void pic_send_eoi(uint8_t irq);
// Функция для включения/выключения IRQ (должна быть в idt.c или system.h)
extern void pic_enable_irq(uint8_t irq);

// Таблица скан-кодов (основные символы для наглядности)
// Это US-раскладка, Set 1
static const unsigned char kbd_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0,  ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  0
};

// Инициализация клавиатуры
void init_keyboard(void) {
    // Включаем IRQ1 (клавиатура)
    pic_enable_irq(1);
    terminal_write_string("Phase 2: Keyboard enabled (IRQ1).\n");
}


// Главный обработчик прерывания клавиатуры (вызывается из ISR)
void keyboard_handler_c(registers_t *regs) {
    // 1. Читаем сканированный код
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // ВРЕМЕННЫЙ ТЕСТ: Выводим, что прерывание сработало
    terminal_write_string("KBD Interrupt: ");
    terminal_write_hex(scancode);
    terminal_put_char('\n');

    // 2. Отправляем подтверждение (EOI) контроллерам PIC
    // Поскольку IRQ 1 — это прерывание от Slave PIC,
    // EOI должно быть отправлено обоим PIC'ам.
    outb(0x20, 0x20); // EOI на Master PIC
    outb(0xA0, 0x20); // EOI на Slave PIC
}


void keyboard_install() {
    // Устанавливаем обработчик для IRQ 1, который соответствует ISR 33
    // (ISR 32 - 47 зарезервированы для IRQ 0 - 15)
    install_interrupt_handler(IRQ1, keyboard_handler_c);
}