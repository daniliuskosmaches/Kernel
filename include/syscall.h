#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"

void syscall_handler(registers_t *regs);
void shutdown();
void reboot();
int sys_read(int fd, void* buffer, size_t count);
int read(int fd, void* buffer, size_t count);
void write();
void malloc();
void exec();
void open();


#endif // SYSCALL_H