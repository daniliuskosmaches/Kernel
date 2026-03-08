//
// Created by nazar on 07.03.26.
//

#include "../../../include/vga.h"
#include "../../../include/keyboard.h"
#include "../../../include/vfs/vfs.h"
#include "../../../include/app/text_editor/piko.h"

void piko_start() {
    terminal_clear_screen();
    terminal_write_string("--- PIKO EDITOR v0.1 | Press F10 to Save & Exit ---\n");

    char buffer[2048]; // Наш текстовый буфер
    int pos = 0;

    while(1) {
        char c = keyboard_get_char(); // Твоя функция чтения клавиши

        if (c == KEY_F10) { // Если нажали F10
            save_to_file("newfile.txt", buffer, pos);
            break;
        } else if (c == KEY_BACKSPACE) {
            if (pos > 0) {
                pos--;
                terminal_backspace(); // Нужно реализовать удаление символа на экране
            }
        } else {
            buffer[pos++] = c;
            terminal_put_char(c);
        }
    }
}

void save_to_file(const char* filename, char* buffer, uint32_t length) {
    // В будущем тут будет vfs_create или vfs_open с флагом O_CREAT
    vfs_node_t* file = vfs_open(filename);

    if (file) {
        vfs_write(file, 0, length, (uint8_t*)buffer);
        // Выведи сообщение через vga: "File saved!"
    } else {
        // Ошибка: файл не найден или ФС только для чтения (как наш будущий initrd)
    }
}