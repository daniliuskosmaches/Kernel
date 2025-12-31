#include "shell.h"
#include "vga.h"
#include "string.h"


#define MAX_COMMAND_LENGTH 256

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_position = 0;

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ (LIBC) ---


static void cmd_help(void) {
    terminal_write_string("Available: list, say, sysinfo, uptime, clear, help\n");
}

static void cmd_clear(void) {
    terminal_clear_screen();
}

static void cmd_echo(const char* args) {
    if (*args == '\0') {
        terminal_write_string("\n");
    } else {
        terminal_write_string(args);
        terminal_write_string("\n");
    }
}

// --- ПАРСЕР И ИСПОЛНЕНИЕ ---

void shell_init(void) {
    command_position = 0;
    memset(command_buffer, 0, MAX_COMMAND_LENGTH);
    terminal_write_string("MyOS Shell v1.1. Type 'help' to start.\n");
    terminal_write_string("myos> ");
}

void shell_execute_command(char* cmd_line) {
    // 1. Пропускаем начальные пробелы
    while (*cmd_line == ' ') cmd_line++;
    if (*cmd_line == '\0') return;

    // 2. Отделяем имя команды от аргументов
    char* args = cmd_line;
    while (*args != ' ' && *args != '\0') {
        args++;
    }

    if (*args == ' ') {
        *args = '\0'; // Завершаем строку с именем команды
        args++;
        while (*args == ' ') args++; // Пропускаем пробелы перед аргументами
    }

    // 3. Сравнение команд через strcmp (безопаснее strncmp)
    if (strcmp(cmd_line, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd_line, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(cmd_line, "say") == 0) {
        cmd_echo(args);
    } else if (strcmp(cmd_line, "sysinfo") == 0) {
        terminal_write_string("OS: MyOS v1.1\nArch: i686-elf\n");
    } else {
        terminal_write_string(cmd_line);
        terminal_write_string(": command not found\n");
    }
}

void shell_handle_char(char c) {
    if (c == '\n') {
        terminal_put_char('\n');
        if (command_position > 0) {
            command_buffer[command_position] = '\0';
            shell_execute_command(command_buffer);
        }
        command_position = 0;
        memset(command_buffer, 0, MAX_COMMAND_LENGTH);
        terminal_write_string("myos> ");
    }
    else if (c == '\b') {
        if (command_position > 0) {
            command_position--;
            terminal_backspace(); // Используем твою функцию из vga.c
        }
    }
    else {
        if (command_position < MAX_COMMAND_LENGTH - 1) {
            terminal_put_char(c);
            command_buffer[command_position++] = c;
        }
    }
}