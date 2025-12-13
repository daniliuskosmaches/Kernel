// src/timer.c - ИСПРАВЛЕННАЯ ВЕРСИЯ

#include "timer.h"
#include "idt.h"
#include "system.h"
#include "task.h"
#include "isr.h"

#define PIT_FREQ 1193180  // Базовая частота PIT (1.193180 МГц)

// Глобальный счетчик тиков
volatile unsigned long timer_ticks = 0;

// Частота таймера (в Гц)
static unsigned long timer_frequency = 0;

// ============================================================
// C-ОБРАБОТЧИК ПРЕРЫВАНИЯ ТАЙМЕРА
// ============================================================
void timer_handler_c(registers_t *regs) {
    (void)regs; // Неиспользуемый параметр

    timer_ticks++;

    // Переключение задач (если многозадачность инициализирована)
    if (current_task != 0) {
        switch_task();
    }

    // EOI отправляется автоматически в isr_handler_c
}

// ============================================================
// ИНИЦИАЛИЗАЦИЯ ТАЙМЕРА
// ============================================================
void time_install(unsigned long frequency) {
    if (frequency == 0 || frequency > PIT_FREQ) {
        frequency = 100; // По умолчанию 100 Гц
    }

    timer_frequency = frequency;

    // Вычисляем делитель для PIT
    unsigned long divisor = PIT_FREQ / frequency;

    // Отправляем команду в PIT Command Register (0x43)
    // Формат: 0x36 = 00110110b
    // - Биты 7-6: 00 = Канал 0
    // - Биты 5-4: 11 = Access Mode: lobyte/hibyte
    // - Биты 3-1: 011 = Operating Mode 3 (Square Wave)
    // - Бит 0:    0 = 16-битный бинарный режим
    outb(0x43, 0x36);

    // Отправляем делитель в Data Port канала 0 (0x40)
    // Сначала младший байт, потом старший
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));

    // Регистрируем обработчик для IRQ0 (0x20)
    register_interrupt_handler(IRQ0, timer_handler_c);

    // Включаем IRQ0 в PIC
    pic_enable_irq(0);
}

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================

// Получить количество тиков с момента запуска
unsigned long timer_get_ticks(void) {
    return timer_ticks;
}

// Простая функция задержки (sleep)
void timer_wait(unsigned long ticks) {
    unsigned long end_tick = timer_ticks + ticks;
    while (timer_ticks < end_tick) {
        __asm__ volatile("hlt"); // Ждем прерывания
    }
}

// Задержка в миллисекундах
void timer_sleep_ms(unsigned long milliseconds) {
    if (timer_frequency == 0) return;

    // Вычисляем количество тиков
    unsigned long ticks = (milliseconds * timer_frequency) / 1000;
    timer_wait(ticks);
}