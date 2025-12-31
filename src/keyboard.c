#include "keyboard.h"
#include "io.h"
#include "vga.h"
#include <stdbool.h>

#define INPUT_MAX 256

// Очередь для скан-кодов (сырые данные от прерывания)
static uint8_t kbd_queue[INPUT_MAX];
static uint32_t head = 0;
static uint32_t tail = 0;

// Буфер для формирования строки текста
static char input_buffer[INPUT_MAX];
static uint32_t input_len = 0;
static bool line_ready = false;

static const char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
    0,'*',0,' ',
};

void keyboard_init() {
    // Регистрация обработчика (IRQ1 -> Вектор 0x21)
    register_interrupt_handler(0x21, keyboard_handler);

    // Разрешаем IRQ1 в контроллере прерываний
    uint8_t mask = inb(0x21);
    outb(0x21, mask & ~(1 << 1));
}

// ВЕРХНЯЯ ПОЛОВИНА: Только чтение порта и запись в очередь
void keyboard_handler() {
    uint8_t scancode = inb(0x60);

    // Проверяем, что это нажатие (бит 7 не установлен)
    if (!(scancode & 0x80)) {
        uint32_t next = (head + 1) % INPUT_MAX;
        if (next != tail) {
            kbd_queue[head] = scancode;
            head = next;
        }
    }

    outb(0x20, 0x20); // EOI для PIC
}

// НИЖНЯЯ ПОЛОВИНА: Обработка символов и вывод на экран
bool keyboard_line_ready() {
    while (head != tail) {
        uint8_t scancode = kbd_queue[tail];
        tail = (tail + 1) % INPUT_MAX;

        char c = keymap[scancode];
        if (!c) continue;

        if (c == '\n') {
            input_buffer[input_len] = 0;
            line_ready = true;
            terminal_put_char('\n');
            return true;
        }
        else if (c == '\b') {
            if (input_len > 0) {
                input_len--;
                // Используем твою функцию из vga.c для визуального стирания
                terminal_put_char('\b');
            }
        }
        else if (input_len < INPUT_MAX - 1) {
            input_buffer[input_len++] = c;
            terminal_put_char(c); // Безопасная печать символа
        }
    }
    return line_ready;
}

void keyboard_get_line(char* out) {
    uint32_t i = 0;
    while (i < input_len) {
        out[i] = input_buffer[i];
        i++;
    }
    out[i] = 0;
    input_len = 0;
    line_ready = false;
}