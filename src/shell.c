// src/shell.c - ПОЛНОЦЕННАЯ КОМАНДНАЯ ОБОЛОЧКА

#include "shell.h"
#include "vga.h"
#include "string.h"
#include "timer.h"

#define MAX_COMMAND_LENGTH 256

// Буфер команды
static char command_buffer[MAX_COMMAND_LENGTH];
static int command_position = 0;

// Прототипы команд
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_echo(const char* args);
static void cmd_meminfo(void);
static void cmd_uptime(void);
static void cmd_version(void);

// ============================================================
// ИНИЦИАЛИЗАЦИЯ SHELL
// ============================================================
void shell_init(void) {
    command_position = 0;
    memset(command_buffer, 0, MAX_COMMAND_LENGTH);

    terminal_write_string("MyOS Shell v1.0\n");
    terminal_write_string("Type 'help' for available commands\n\n");
    shell_print_prompt();
}

// ============================================================
// ВЫВОД ПРИГЛАШЕНИЯ
// ============================================================
void shell_print_prompt(void) {
    terminal_write_string("myos> ");
}

// ============================================================
// ОБРАБОТКА СИМВОЛА ОТ КЛАВИАТУРЫ
// ============================================================
void shell_handle_char(char c) {
    if (c == '\n') {
        // Enter - выполняем команду
        terminal_put_char('\n');

        if (command_position > 0) {
            command_buffer[command_position] = '\0';
            shell_execute_command(command_buffer);

            // Очищаем буфер
            command_position = 0;
            memset(command_buffer, 0, MAX_COMMAND_LENGTH);
        }

        shell_print_prompt();

    } else if (c == '\b') {
        // Backspace - удаляем последний символ
        if (command_position > 0) {
            command_position--;
            command_buffer[command_position] = '\0';
            terminal_backspace();
        }

    } else if (c >= 32 && c < 127) {
        // Обычный символ
        if (command_position < MAX_COMMAND_LENGTH - 1) {
            command_buffer[command_position++] = c;
            terminal_put_char(c);
        }
    }
}

// ============================================================
// ВЫПОЛНЕНИЕ КОМАНДЫ
// ============================================================
void shell_execute_command(const char* command) {
    // Пропускаем пробелы в начале
    while (*command == ' ') command++;

    if (strlen(command) == 0) {
        return;
    }

    // Разделяем команду и аргументы
    const char* args = command;
    while (*args && *args != ' ') args++;

    int cmd_len = args - command;

    // Пропускаем пробелы перед аргументами
    while (*args == ' ') args++;

    // Сравниваем команды
    if (cmd_len == 4 && strncmp(command, "help", 4) == 0) {
        cmd_help();
    }
    else if (cmd_len == 5 && strncmp(command, "clear", 5) == 0) {
        cmd_clear();
    }
    else if (cmd_len == 4 && strncmp(command, "echo", 4) == 0) {
        cmd_echo(args);
    }
    else if (cmd_len == 7 && strncmp(command, "meminfo", 7) == 0) {
        cmd_meminfo();
    }
    else if (cmd_len == 6 && strncmp(command, "uptime", 6) == 0) {
        cmd_uptime();
    }
    else if (cmd_len == 7 && strncmp(command, "version", 7) == 0) {
        cmd_version();
    }
    else {
        terminal_write_string("Unknown command: ");
        for (int i = 0; i < cmd_len; i++) {
            terminal_put_char(command[i]);
        }
        terminal_write_string("\nType 'help' for available commands\n");
    }
}

// ============================================================
// КОМАНДЫ
// ============================================================

static void cmd_help(void) {
    terminal_write_string("Available commands:\n");
    terminal_write_string("  help      - Show this help message\n");
    terminal_write_string("  clear     - Clear the screen\n");
    terminal_write_string("  echo      - Echo text to screen\n");
    terminal_write_string("  meminfo   - Display memory information\n");
    terminal_write_string("  uptime    - Show system uptime\n");
    terminal_write_string("  version   - Show OS version\n");
}

static void cmd_clear(void) {
    terminal_clear_screen();
}

static void cmd_echo(const char* args) {
    if (strlen(args) > 0) {
        terminal_write_string(args);
    }
    terminal_put_char('\n');
}

extern unsigned int total_pages;
extern unsigned int used_pages;

static void cmd_meminfo(void) {
    terminal_write_string("Memory Information:\n");
    terminal_write_string("  Total pages: ");
    terminal_write_hex(total_pages);
    terminal_write_string("\n  Used pages:  ");
    terminal_write_hex(used_pages);
    terminal_write_string("\n  Free pages:  ");
    terminal_write_hex(total_pages - used_pages);

    unsigned int total_kb = (total_pages * 4096) / 1024;
    unsigned int used_kb = (used_pages * 4096) / 1024;
    unsigned int free_kb = total_kb - used_kb;

    terminal_write_string("\n  Total memory: ");
    terminal_write_hex(total_kb);
    terminal_write_string(" KB\n  Used memory:  ");
    terminal_write_hex(used_kb);
    terminal_write_string(" KB\n  Free memory:  ");
    terminal_write_hex(free_kb);
    terminal_write_string(" KB\n");
}

static void cmd_uptime(void) {
    unsigned long ticks = timer_get_ticks();
    unsigned long seconds = ticks / 100; // Assuming 100 Hz timer
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;

    seconds %= 60;
    minutes %= 60;

    terminal_write_string("Uptime: ");
    terminal_write_hex(hours);
    terminal_write_string("h ");
    terminal_write_hex(minutes);
    terminal_write_string("m ");
    terminal_write_hex(seconds);
    terminal_write_string("s\n");
}

static void cmd_version(void) {
    terminal_write_string("MyOS v1.0\n");
    terminal_write_string("A simple educational operating system\n");
    terminal_write_string("Built with love and assembly\n");
}

// Вспомогательная функция для strncmp
int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}