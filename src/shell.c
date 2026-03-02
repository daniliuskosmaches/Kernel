#include "../include/app/shell.h"
#include "../include/vga.h"
#include "../include/lib/string.h"

void shell_init(void) {
    terminal_write_string("Welcome to MyOS Shell!\n");
    terminal_write_string("Type 'help' for a list of commands.\n");
    terminal_write_string("> ");
}



void shell_handle_char(char c) {
    static char input_buffer[128];
    static int input_len = 0;

    if (c == '\n') {
        input_buffer[input_len] = 0; // Завершаем строку
        shell_execute_command(input_buffer);
        input_len = 0; // Сброс буфера
    }
    else if (c == '\b') {
        if (input_len > 0) {
            input_len--;
            terminal_put_char('\b'); // Визуально удаляем символ
        }
    }
    else if (input_len < sizeof(input_buffer) - 1) {
        input_buffer[input_len++] = c;
        terminal_put_char(c); // Печатаем символ
    }
}

void shell_execute_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        terminal_write_string("Available commands:\n");
        terminal_write_string(" - help: Show this message\n");
        terminal_write_string(" - clear: Clear the screen\n");
        terminal_write_string(" - mem: Show memory usage\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        terminal_clear_screen();
    }
    else if (strcmp(cmd, "mem") == 0) {
        // Здесь можно вызвать функцию из pmm.c для отображения состояния памяти
        // Например: pmm_print_status();
        terminal_write_string("Memory status command not implemented yet.\n");
    }
    else {
        terminal_write_string("Unknown command: ");
        terminal_write_string(cmd);
        terminal_write_string("\n");
    }
}