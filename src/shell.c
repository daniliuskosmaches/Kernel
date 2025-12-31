#include "shell.h"
#include <stdbool.h>
#include "vga.h"
#include "string.h"

static char cmd_buffer[256];

void shell_init() {
    terminal_write_string("Simple kernel shell v1.2\n");
    terminal_write_string("myos> ");
}

static void execute_command(const char* cmd) {
    if (strlen(cmd) == 0) return;

    if (strcmp(cmd, "help") == 0) {
        terminal_write_string("Commands: help, clear, info, hex\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        terminal_clear_screen();
    }
    else if (strcmp(cmd, "info") == 0) {
        terminal_write_string("OS: MyOS | Kernel: i686-elf\n");
    }
    else if (strcmp(cmd, "hex") == 0) {
        terminal_write_string("VGA Buffer Start: ");
        terminal_write_hex(0xB8000);
        terminal_write_string("\n");
    }
    else {
        // Та самая ошибка, которую ты хотел
        terminal_write_string("command not found: ");
        terminal_write_string(cmd);
        terminal_put_char('\n');
    }
}


bool keyboard_line_ready() {
    while (head != tail) {
        uint8_t scancode = kbd_queue[tail];
        tail = (tail + 1) % INPUT_MAX;

        char c = keymap[scancode];
        if (!c) continue;

        if (c == '\n') {
            input_buffer[input_len] = 0; // Закрываем строку
            line_ready = true;
            terminal_put_char('\n');
            return true;
        }
        else if (c == '\b') {
            if (input_len > 0) {
                input_len--;
                // Твоя функция из vga.c, которая сама сотрет символ
                terminal_put_char('\b');
            }
        }
        else if (input_len < INPUT_MAX - 1) {
            input_buffer[input_len++] = c;
            terminal_put_char(c); // Безопасно печатаем символ (мы не в прерывании!)
        }
    }
    return line_ready;
}

// 4. ПОЛУЧЕНИЕ СТРОКИ
// Копирует накопленный буфер в строку шелла и сбрасывает флаги
void keyboard_get_line(char* out) {
    uint32_t i = 0;
    while (i < input_len) {
        out[i] = input_buffer[i];
        i++;
    }
    out[i] = 0; // Завершающий нуль

    input_len = 0;
    line_ready = false;
}

void shell_loop() {
    while (1) {
        // Опрашиваем клавиатуру (это происходит в основном цикле, не в прерывании!)
        if (!keyboard_line_ready()) {
            __asm__ volatile("hlt"); // Экономим CPU до следующего нажатия
            continue;
        }

        // Забираем строку
        keyboard_get_line(cmd_buffer);

        // Выполняем
        execute_command(cmd_buffer);

        // Печатаем промпт
        terminal_write_string("myos> ");
    }
}

