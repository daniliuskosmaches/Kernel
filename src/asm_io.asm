; src/asm_io.asm

section .text
global inb
global outb
global lidt

; ----------------------------------------------------
; Функция inb (ввод байта):
; unsigned char inb(unsigned short port);
; Вход: порт (DX), Выход: байт (AL)
inb:
    mov dx, [esp+4] ; Загружаем 'port' (первый аргумент C) в регистр DX
    in al, dx       ; Читаем байт из порта DX в регистр AL
    ret             ; AL возвращается как результат функции

; ----------------------------------------------------
; Функция outb (вывод байта):
; void outb(unsigned short port, unsigned char data);
; Вход: порт (DX), данные (AL)
outb:
    mov dx, [esp+4] ; Загружаем 'port' в регистр DX
    mov al, [esp+8] ; Загружаем 'data' (второй аргумент C) в регистр AL
    out dx, al      ; Записываем байт из AL в порт DX
    ret

; ----------------------------------------------------
; Функция lidt (Загрузка IDT):
; void lidt(void *idtr_ptr);
; Вход: указатель на idt_ptr (IDTR)
lidt:
    mov eax, [esp+4] ; Загружаем адрес idt_ptr в регистр EAX
    lidt [eax]       ; Загружаем IDT-указатель (содержимое EAX) в регистр IDTR
    ret