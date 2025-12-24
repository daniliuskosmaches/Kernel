#include "shell.h"
#include "vga.h"
#include "string.h"
#include "timer.h"

#define MAX_COMMAND_LENGTH 256

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_position = 0;

// --- ПРОТОТИПЫ (Чтобы компилятор не ругался) ---
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_echo(const char* args);
static void cmd_list(void);
static void cmd_sysinfo(void);
static void cmd_uptime(void);

// Функция для вывода ошибки в стиле Linux
static void print_not_found(const char* cmd, int len) {
    for (int i = 0; i < len; i++) {
        terminal_put_char(cmd[i]);
    }
    terminal_write_string(": command not found\n");
}

void shell_init(void) {
    command_position = 0;
    memset(command_buffer, 0, MAX_COMMAND_LENGTH);
    terminal_write_string("MyOS Shell v1.0. Type 'help' to start.\n");
    terminal_write_string("myos> ");
}

void shell_handle_char(char c) {
    if (c == '\n') {
        terminal_put_char('\n');
        if (command_position > 0) {
            command_buffer[command_position] = '\0';
            shell_execute_command(command_buffer);
            command_position = 0;
            memset(command_buffer, 0, MAX_COMMAND_LENGTH);
        }
        terminal_write_string("myos> ");
    } else if (c == '\b' && command_position > 0) {
        command_position--;
        terminal_backspace();
    } else if (c >= 32 && c < 127 && command_position < MAX_COMMAND_LENGTH - 1) {
        command_buffer[command_position++] = c;
        terminal_put_char(c);
    }
}

void shell_execute_command(const char* command) {
    while (*command == ' ') command++;
    if (strlen(command) == 0) return;

    const char* args = command;
    while (*args && *args != ' ') args++;
    int cmd_len = args - command;
    while (*args == ' ') args++;

    // Проверка команд
    if (strncmp(command, "help", cmd_len) == 0 && cmd_len == 4) {
        cmd_help();
    } else if (strncmp(command, "clear", cmd_len) == 0 && cmd_len == 5) {
        cmd_clear();
    } else if (strncmp(command, "say", cmd_len) == 0 && cmd_len == 3) {
        cmd_echo(args);
    } else if (strncmp(command, "list", cmd_len) == 0 && cmd_len == 4) {
        cmd_list();
    } else if (strncmp(command, "sysinfo", cmd_len) == 0 && cmd_len == 7) {
        cmd_sysinfo();
    } else if (strncmp(command, "uptime", cmd_len) == 0 && cmd_len == 6) {
        cmd_uptime();
    } else {
        print_not_found(command, cmd_len);
    }
}

// --- РЕАЛИЗАЦИЯ КОМАНД ---

static void cmd_help(void) {
    terminal_write_string("Available commands: list, say, sysinfo, uptime, clear, help\n");
}

static void cmd_clear(void) {
    terminal_clear_screen();
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
    unsigned long t = timer_get_ticks();
    terminal_write_string("Uptime ticks: ");
    terminal_write_hex(t); // Если есть функция вывода hex или int
    terminal_write_string("\n");
}

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