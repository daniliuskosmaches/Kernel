#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"

void syscall_handler(registers_t *regs);
void shutdown();

#endif // SYSCALL_H