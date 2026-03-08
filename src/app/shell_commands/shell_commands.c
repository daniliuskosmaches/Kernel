//
// Created by nazar on 05.03.26.
//
#include "../../../include/vga.h"
#include "../../../include/app/shell_commands.h"
#include "../../../include/lib/stdio.h"
#include "../../../include/vfs/vfs.h"
#include "../../../include/commands.h"



void help(const char* cmd) {
    if (cmd && cmd[0]) {
        terminal_write_string("No detailed help available for this command.\n");
        return;
    }
    terminal_write_string("Available commands:\n");
    terminal_write_string("help - Show this help message\n");
    terminal_write_string("clear - Clear the screen\n");
    terminal_write_string("memspy <address> <size> - Dump memory at address\n");
    terminal_write_string("tasks - List running tasks\n");
    terminal_write_string("uptime - Show system uptime\n");
}
void clear(const char* cmd) {
    terminal_clear_screen();
}
void memspy(const char* cmd) {
    uint32_t address = 0;
    size_t size = 0;
    if (sscanf(cmd, "%x %zu", &address, &size) != 2) {
        terminal_write_string("Usage: memspy <address> <size>\n");
        return;
    }
    terminal_dump_memory(address, size);
}
void tasks(const char* cmd) {
    // Здесь нужно реализовать вывод списка задач из твоего планировщика
    terminal_write_string("Task listing not implemented yet.\n");
}
void uptime(const char* cmd) {
    // Здесь нужно реализовать вывод времени работы системы
    terminal_write_string("Uptime not implemented yet.\n");
}
void reboot(const char* cmd) {
    // Здесь нужно реализовать перезагрузку системы
    terminal_write_string("Reboot not implemented yet.\n");
}
void shutdown(const char* cmd) {
    // Здесь нужно реализовать выключение системы
    terminal_write_string("Shutdown not implemented yet.\n");
}


void ping(const char* cmd) {

}


// Команда 'sh' (Show / ls)
void cmd_sh() {
    terminal_write_string("Contents of ");
    terminal_write_string(current_dir->name);
    terminal_write_string(":\n");
    ramfs_list(current_dir); // Функция из ramfs.c, которую мы обсуждали
}

// Команда 'at' (At / cd)
void cmd_at(char* name) {
    vfs_node_t* target = vfs_find_in_dir(current_dir, name);
    if (target && (target->flags & VFS_DIRECTORY)) {
        current_dir = target;
    } else {
        terminal_write_string("Can't go there.\n");
    }
}

// Команда 'mk' (Make / touch)
void cmd_mk(char* name) {
    ramfs_create(current_dir, name, VFS_FILE);
    terminal_write_string("Entity spawned.\n");
}
void cmd_rm(char* name) {

    terminal_write_string("rm: ");
    terminal_write_string(name);
    terminal_write_string(" - target destroyed (simulated).\n");



}




