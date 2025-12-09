//
// Created by nazar on 09.12.25.
//
// src/timer.c
#define PIT_FREQ 1193180
#include "idt.h" // Для outb
 // Для отладочного вывода

unsigned long timer_ticks = 0;

void timer_handler() {
    timer_ticks++;

    // Примечание: Эта функция вызывается 18.2 раза в секунду по умолчанию.
    // Если мы настроим ее на 100 Гц (как в Шаге 2), она будет вызываться 100 раз.

    // Для отладки (выводим 'T' раз в секунду)
    if (timer_ticks % 100 == 0) {
        // Здесь можно выводить время или отладочную информацию
        // Например:
        // terminal_write_string("Tick.");
    }
}

void timer_install(int frequency) {
    // 1. Рассчитываем делитель
    int divisor = PIT_FREQ / frequency;

    // 2. Отправляем команду в PIT (порт 0x43)
    // 0x36: Канал 0, Режим 3 (Square Wave), LOBYTE/HIBYTE
    outb(0x43, 0x36);

    // 3. Отправляем делитель (младший байт) в Канал 0 (порт 0x40)
    outb(0x40, divisor & 0xFF);

    // 4. Отправляем делитель (старший байт) в Канал 0
    outb(0x40, (divisor >> 8) & 0xFF);
}