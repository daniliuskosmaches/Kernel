#include "../../include/app/shell.h"
#include "../../include/vga.h"
#include "../../include/lib/string.h"
#include "../../include/app/shell_commands.h"
#include "../../include/vfs/vfs.h"




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
    current_dir = vfs_root;
    if (current_dir == NULL) {
        terminal_write_string("SYSTEM HALT: current_dir is NULL!\n");
        for(;;);
    }
    terminal_clear_screen();
    terminal_set_color(0x0B);
    terminal_write_string("Welcome to MyOS (Linux-like terminal)!\n");
    terminal_set_color(0x07);
    shell_print_prompt();
}

void shell_execute_command(char* cmd) {
    if (strncmp(cmd, "help", 4) == 0) {
        help(cmd + 4);
    }





    if (strncmp(cmd, "at ", 3) == 0) {       // Зайти в папку (вместо cd)
        cmd_at(cmd + 3);
    }
    else if (strncmp(cmd, "sh", 2) == 0) {   // Показать файлы (вместо ls)
        cmd_sh();
    }
    else if (strncmp(cmd, "mk ", 3) == 0) {  // Создать (вместо touch/mkdir)
        cmd_mk(cmd + 3);
    }
    else if (strncmp(cmd, "ed ", 3) == 0) {  // Редактировать (открыть piko)
        piko_start(cmd + 3);
    }
    else if (strncmp(cmd, "rm ", 3) == 0) {  // Удалить
        cmd_rm(cmd + 3);
    }
    // Твои старые команды
    else if (strncmp(cmd, "help", 4) == 0) {
        help(cmd + 4);
    }
    else if (strncmp(cmd, "clear", 5) == 0) {
        terminal_initialize(); // Просто сброс экрана
    }
    else if (strncmp(cmd, "ping", 4) == 0) {
        ping(cmd + 4);
    }
    else if (strncmp(cmd, "reboot", 6) == 0) {
        reboot(cmd + 6);
    }
    else {
        terminal_write_string("Unknown: ");
        terminal_write_string(cmd);
        terminal_write_string("\n");
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