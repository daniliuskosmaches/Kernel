// src/keyboard.c - ВИПРАВЛЕНА ВЕРСІЯ

#include "keyboard.h"
#include "vga.h"
#include "system.h"
#include "stdint.h"
#include "idt.h"
#include "isr.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Таблиці раскладок клавіатури (US, Scan Code Set 1)
static const unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0,  ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0
};

static const unsigned char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',
    0,  ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,  '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0
};

// Стан модифікаторів
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t caps_lock = 0;

// Зовнішня функція для передачі символів в shell
extern void shell_handle_char(char c);

// ===== ІНІЦІАЛІЗАЦІЯ КЛАВІАТУРИ =====
void init_keyboard(void) {
    terminal_write_string("Initializing keyboard...\n");

    // 1. Реєструємо обробник для IRQ1 (вектор 0x21 = 32 + 1)
    register_interrupt_handler(IRQ1, keyboard_handler_c);

    // 2. Розблокуємо IRQ1 в PIC
    pic_enable_irq(1);

    // 3. ТЕПЕР вмикаємо переривання глобально
    __asm__ volatile("sti");

    terminal_write_string("Keyboard initialized. Interrupts enabled.\n");
}

// ===== ГОЛОВНИЙ ОБРОБНИК ПРЕРЫВАННЯ КЛАВІАТУРИ =====
void keyboard_handler_c(registers_t *regs) {
    (void)regs;

    // Читаємо скан-код
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // ОБРОБКА ВІДПУСКАННЯ КЛАВІШ (Release)
    if (scancode & 0x80) {
        scancode &= 0x7F;

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 0;
        } else if (scancode == 0x1D) {
            ctrl_pressed = 0;
        } else if (scancode == 0x38) {
            alt_pressed = 0;
        }

        return;
    }

    // ОБРОБКА НАТИСКАННЯ МОДИФІКАТОРІВ
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }

    if (scancode == 0x1D) {
        ctrl_pressed = 1;
        return;
    }

    if (scancode == 0x38) {
        alt_pressed = 1;
        return;
    }

    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    // ПЕРЕТВОРЕННЯ СКАН-КОДУ В СИМВОЛ
    char c = 0;

    if (scancode < 128) {
        if (shift_pressed) {
            c = kbd_us_shift[scancode];
        } else {
            c = kbd_us[scancode];
        }

        // Застосовуємо Caps Lock
        if (caps_lock) {
            if (c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            } else if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 'a';
            }
        }
    }

    // ОБРОБКА СИМВОЛУ
    if (c != 0) {
        // Спеціальні комбінації
        if (ctrl_pressed && c == 'c') {
            terminal_write_string("^C\n");
            return;
        }

        if (ctrl_pressed && c == 'l') {
            terminal_clear_screen();
            return;
        }

        if (ctrl_pressed && c == 'd') {
            terminal_write_string("^D\n");
            return;
        }

        // Передаємо в shell
        shell_handle_char(c);
    }

    // EOI відправляється автоматично в isr_handler_c
}

void keyboard_install(void) {
    init_keyboard();
}