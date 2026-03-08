//
// Created by nazar on 07.03.26.
//

#ifndef UNTITLED2_SHELL_COMMANDS_H
#define UNTITLED2_SHELL_COMMANDS_H

void help(const char* cmd);
void clear(const char* cmd);
void memspy(const char* cmd);
void tasks(const char* cmd);
void uptime(const char* cmd);
void reboot(const char* cmd);
void shutdown(const char* cmd);
void ping(const char* cmd);

void cmd_at(char* path);
void cmd_sh();
void cmd_mk(char* name);
void cmd_rm(char* name);

// Текстовый редактор
void piko_start(char* filename);

#endif //UNTITLED2_SHELL_COMMANDS_H