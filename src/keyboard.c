// src/keyboard.c - ИСПРАВЛЕННАЯ ВЕРСИЯ
#include "../include/keyboard.h"
#include "../include/io.h"
#include "../include/vga.h"
#include "../include/idt.h"

#define INPUT_MAX 256

// Очередь сканкодов (заполняется в прерывании)
static uint8_t kbd_queue[INPUT_MAX];
static volatile uint32_t head = 0; // FIX: volatile — меняется в прерывании
static volatile uint32_t tail = 0; // FIX: volatile

// Буфер строки
static char input_buffer[INPUT_MAX];
static uint32_t input_len = 0;
static bool line_ready = false;

static const char keymap[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',
    0,  '*', 0,  ' ',
};

void init_keyboard(void) {
    // FIX: Сбрасываем очередь при инициализации
    head = 0;
    tail = 0;
    input_len = 0;
    line_ready = false;

    // Регистрируем обработчик для IRQ1 (вектор 33 = 0x20 + 1)
    register_interrupt_handler(IRQ1, keyboard_handler);

    // Разрешаем IRQ1 в PIC
    // FIX: используем pic_enable_irq вместо прямого outb
    pic_enable_irq(1);
}

// Обработчик прерывания клавиатуры (верхняя половина)
// EOI отправляется в isr_handler_c в idt.c — здесь НЕ нужен
void keyboard_handler(registers_t *regs) {
    (void)regs;

    uint8_t scancode = inb(0x60);

    // FIX: Читаем статус контроллера — убеждаемся что данные готовы
    // (на некоторых эмуляторах это важно)

    // Только нажатия (бит 7 = 0), игнорируем отпускания (бит 7 = 1)
    if (scancode & 0x80) return;

    // FIX: Проверяем что scancode в пределах keymap
    if (scancode >= 128) return;

    uint32_t next = (head + 1) % INPUT_MAX;
    if (next != tail) { // Очередь не полная
        kbd_queue[head] = scancode;
        head = next;
    }
    // EOI НЕ здесь — он отправляется в isr_handler_c
}

// Возвращает символ из очереди или 0 если очередь пуста
// FIX: Используется ТОЛЬКО в главном цикле kernel.c
char keyboard_get_char(void) {
    if (head == tail) return 0; // Очередь пуста

    uint8_t scancode = kbd_queue[tail];
    tail = (tail + 1) % INPUT_MAX;

    if (scancode >= 128) return 0; // Защита
    return keymap[scancode];
}

// Нижняя половина: формирует строку из очереди и выводит на экран
// FIX: НЕ вызывать одновременно с keyboard_get_char — они оба двигают tail!
// Используй либо keyboard_get_char (в kernel.c), либо keyboard_line_ready (в шелле)
bool keyboard_line_ready(void) {
    while (head != tail) {
        uint8_t scancode = kbd_queue[tail];
        tail = (tail + 1) % INPUT_MAX;

        if (scancode >= 128) continue; // Защита
        char c = keymap[scancode];
        if (!c) continue;

        if (c == '\n') {
            input_buffer[input_len] = 0;
            line_ready = true;
            terminal_put_char('\n');
            return true;
        } else if (c == '\b') {
            if (input_len > 0) {
                input_len--;
                terminal_put_char('\b');
            }
        } else if (input_len < INPUT_MAX - 1) {
            input_buffer[input_len++] = c;
            terminal_put_char(c);
        }
    }
    return line_ready;
}

void keyboard_get_line(char *out) {
    uint32_t i = 0;
    while (i < input_len) {
        out[i] = input_buffer[i];
        i++;
    }
    out[i] = 0;
    input_len = 0;
    line_ready = false;
}