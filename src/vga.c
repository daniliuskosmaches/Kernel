//
// Created by nazar on 09.12.25.
//
// src/vga.c

// Адрес видеопамяти VGA Text Mode
#define VGA_ADDRESS 0xB8000
unsigned short *vga_buffer = (unsigned short *)VGA_ADDRESS;
unsigned int vga_col = 0;
unsigned int vga_row = 0;

void terminal_put_char(char c) {
    if (c == '\n') {
        vga_row++;
        vga_col = 0;
    } else {
        // Создаем символ: ASCII код | (цвет << 8)
        unsigned short entry = c | (0x0F << 8); // 0x0F = Белый на черном

        // Записываем символ в текущую позицию
        vga_buffer[vga_row * 80 + vga_col] = entry;

        vga_col++;
        if (vga_col >= 80) {
            vga_row++;
            vga_col = 0;
        }
    }

    // Простая прокрутка (если строка выходит за пределы 25 строк)
    if (vga_row >= 25) {
        // (Для простоты, здесь должна быть логика прокрутки,
        // пока просто сбросим курсор на начало)
        vga_row = 0;
        vga_col = 0;
    }
}