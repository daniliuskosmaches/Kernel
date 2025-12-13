// src/isr.h (НЕ КОНФЛИКТУЕТ)

#ifndef ISR_H
#define ISR_H

#include <stdint.h>

// --- Структура регистров (Контекст, сохраненный isr.asm) ---
typedef struct registers {
    uint32_t ds;        // Сегмент данных
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// Прототип главного C-обработчика
void isr_handler_c(registers_t *regs, uint8_t int_no);


#endif // ISR_H