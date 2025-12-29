#include "shell.h"
#include "vga.h"
#include "string.h"
#include "timer.h"
#include <stddef.h>

#define MAX_COMMAND_LENGTH 256

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_position = 0;

// --- РЕАЛИЗАЦИЯ КОМАНД ---

static void cmd_help(void) {
    terminal_write_string("Available commands: list, say, sysinfo, uptime, clear, help\n");
}

static void cmd_clear(void) {
    terminal_initialize();
}

static void cmd_echo(const char* args) {
    terminal_write_string(args);
    terminal_write_string("\n");
}

static void cmd_list(void) {
    terminal_write_string("bin/  dev/  etc/  usr/\n");
}

static void cmd_sysinfo(void) {
    terminal_write_string("OS: MyOS v1.0\nArch: i686-elf\n");
}

static void cmd_uptime(void) {
    terminal_write_string("Uptime functionality active.\n");
}

static void print_not_found(const char* command) {
    terminal_write_string(command);
    terminal_write_string(": command not found\n");
}

// --- ОСНОВНАЯ ЛОГИКА ---

void shell_init(void) {
    command_position = 0;
    memset(command_buffer, 0, MAX_COMMAND_LENGTH);
    terminal_write_string("MyOS Shell v1.0. Type 'help' to start.\n");
    terminal_write_string("myos> ");
}

// Убрали const, чтобы соответствовать логике парсера
void shell_execute_command(char* command) {
    while (*command == ' ') command++;
    if (*command == '\0') return;

    char* end_of_cmd = command;
    while (*end_of_cmd && *end_of_cmd != ' ') end_of_cmd++;

    char saved_char = *end_of_cmd;
    *end_of_cmd = '\0'; // Разделяем команду и аргументы

    char* args = (saved_char == ' ') ? end_of_cmd + 1 : end_of_cmd;
    while (*args == ' ') args++;

    if (strncmp(command, "help", 4) == 0) {
        cmd_help();
    } else if (strncmp(command, "clear", 5) == 0) {
        cmd_clear();
    } else if (strncmp(command, "say", 3) == 0) {
        cmd_echo(args);
    } else if (strncmp(command, "list", 4) == 0) {
        cmd_list();
    } else if (strncmp(command, "sysinfo", 7) == 0) {
        cmd_sysinfo();
    } else if (strncmp(command, "uptime", 6) == 0) {
        cmd_uptime();
    } else {
        print_not_found(command);
    }
}

void shell_handle_char(char c) {
    if (c == '\n') {
        terminal_put_char('\n');
        if (command_position > 0) {
            command_buffer[command_position] = '\0';
            shell_execute_command(command_buffer); //
        }
        command_position = 0;
        memset(command_buffer, 0, MAX_COMMAND_LENGTH);
        terminal_write_string("myos> ");
    }
    else if (c == '\b') {
        if (command_position > 0) {
            command_position--;
            terminal_backspace(); //
        }
    }
    else {
        if (command_position < MAX_COMMAND_LENGTH - 1) {
            terminal_put_char(c);
            command_buffer[command_position++] = c;
        }
    }
}

// Реализация strncmp
int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

