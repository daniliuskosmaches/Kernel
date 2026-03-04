#include "../include/app/shell.h"
#include "../include/vga.h"
#include "../include/lib/string.h"

#define PROMPT_TEXT "user@myos:/$ "
#define PROMPT_LEN 12

void shell_print_prompt() {
    terminal_set_color(0x0A); // Зеленый для пользователя
    terminal_write_string("user@myos");
    terminal_set_color(0x07); // Серый
    terminal_write_string(":");
    terminal_set_color(0x09); // Синий для пути
    terminal_write_string("/");
    terminal_set_color(0x0F); // Белый
    terminal_write_string("$ ");

    // Устанавливаем лимит в VGA, чтобы не стереть промпт
    terminal_set_limit(PROMPT_LEN);
}

void shell_init(void) {
    terminal_clear_screen();
    terminal_set_color(0x0B);
    terminal_write_string("Welcome to MyOS (Linux-like terminal)!\n");
    terminal_set_color(0x07);
    shell_print_prompt();
}

void shell_execute_command(char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        terminal_write_string("Commands: help, clear, echo [text], color [0-9]\n");
    } else if (strcmp(cmd, "clear") == 0) {
        terminal_clear_screen();
    } else if (strlen(cmd) > 5 && strncmp(cmd, "echo ", 5) == 0) {
        terminal_write_string(cmd + 5);
        terminal_write_string("\n");
    } else if (strlen(cmd) > 0) {
        terminal_set_color(0x0C); // Красный для ошибки
        terminal_write_string("Unknown command: ");
        terminal_write_string(cmd);
        terminal_write_string("\n");
        terminal_set_color(0x0F);
    }
    shell_print_prompt();
}

void shell_handle_char(char c) {
    static char input_buffer[128];
    static int input_len = 0;

    if (c == '\n') {
        terminal_put_char('\n');
        input_buffer[input_len] = '\0';
        shell_execute_command(input_buffer);
        input_len = 0;
    } else if (c == '\b') {
        if (input_len > 0) {
            input_len--;
            terminal_put_char('\b');
        }
    } else if (input_len < 127 && c >= 32) { // Только печатные символы
        input_buffer[input_len++] = c;
        terminal_put_char(c);
    }
}