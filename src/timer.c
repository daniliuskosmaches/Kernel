// src/timer.c - ИСПРАВЛЕННАЯ ВЕРСИЯ

#include "../include/timer.h"
#include "../include/idt.h"
#include "../include/system.h"
#include "../include/isr.h"



#define PIT_FREQ 1193180
volatile unsigned long timer_ticks = 0;
static unsigned long timer_frequency = 0;

void timer_handler_c(registers_t *regs) {
    (void)regs;
    timer_ticks++;

    // ВАЖНО: Пока мы отлаживаем Triple Fault, лучше не переключать задачи
    // автоматически на каждом тике. Сначала проверим стабильность.
    /*
    if (current_task != 0) {
        scheduler_run();
    }
    */
}

void time_install(unsigned long frequency) {
    if (frequency == 0) frequency = 100;
    timer_frequency = frequency;

    unsigned long divisor = PIT_FREQ / frequency;

    outb(0x43, 0x36);
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));

    // Регистрируем обработчик
    register_interrupt_handler(IRQ0, timer_handler_c);
    pic_enable_irq(0);
}

unsigned long timer_get_ticks(void) { return timer_ticks; }

void timer_wait(unsigned long ticks) {
    unsigned long end_tick = timer_ticks + ticks;
    while (timer_ticks < end_tick) {
        __asm__ volatile("hlt");
    }
}