#include "syscall.h"
#include "isr.h"
#include "vga.h"



void syscall_handler(registers_t *regs) {
    // Если в EAX передали 1 — это системный вызов WRITE
    if (regs->eax == 1) {
        terminal_write_string((char*)regs->ebx); // Сама строка в EBX
    }
}