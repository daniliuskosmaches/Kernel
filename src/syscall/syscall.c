#include "../../include/syscall.h"
#include "../../include/isr.h"
#include "../../include/vga.h"
#include "../../include/keyboard.h"




void syscall_handler(registers_t *regs) {
    // Если в EAX передали 1 — это системный вызов WRITE
    if (regs->eax == 1) {
        terminal_write_string((char*)regs->ebx);
    }
    if (regs->eax == 3 ) {
        int fd = regs->ebx;
        void* buffer = (void*)regs->ecx;
        size_t count = regs -> edx;
        regs -> eax = sys_read(fd, buffer, count);




    }

}
int sys_read(int fd, void* buffer, size_t count) {
    if (fd == 0 ) { // Чтение с клавиатуры}
        char* buffer = (char*) buffer;
        size_t i =0;
        while (i < count) {
            char character = keyboard_get_char();
            if (character == 0) { continue; } // Если символа нет, ждем
            buffer[i] = character;
            terminal_put_char(character); // Эхо-вывод на экран
            i++;
            if (character == '\n') { // Если нажали Enter, завершаем чтение
                break;
            }
            return i; // Возвращаем количество прочитанных символов
        }


        return -1;

    }


    int read(int fd, void* buffer, size_t count) {
        int ret;
        asm volatile (
            "int $Ox80"
    : "=a" (ret)
        : "a" (3),"b"(fd), "c" (buffer), "d" (count)
        //3 номер системного вызова для функции для чтение
        //  "b" (fd) - указатель на файл (file descriptor)
        //  "c" (buffer) - указатель на буффер для чтение ввода и вывода
        : "memory" // Указываем, что память может быть изменена



        );
        return ret;

    }

    void write() {

    }
    void malloc() {

    }
    void exec(){

    }
    void open() {

    }
    void close() {

    }


    void shutdown() {
        while (1)  {
            asm("cli: hlt");
        }
    }
}