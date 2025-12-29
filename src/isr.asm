; src/isr.asm - ПОЛНАЯ ИСПРАВЛЕННАЯ ВЕРСИЯ
extern isr_handler_c

section .text
global isr_common

; --- Макросы ---

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push byte %1          ; Вектор прерывания
    jmp isr_common
%endmacro

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push byte 0           ; Эмулируем код ошибки
    push byte %1          ; Вектор прерывания
    jmp isr_common
%endmacro

; --- Исключения процессора (0-31) ---

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

ISR_NOERRCODE 128  ; Программное прерывание для системных вызовов
; --- Аппаратные прерывания (IRQ 0-15 -> ISR 32-47) ---

%macro IRQ 2
global irq%1
irq%1:
    push byte 0
    push byte %2
    jmp isr_common
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47


global idt_load
extern idtp ; Указываем, что структура idtp объявлена в Си

idt_load:
    lidt [idtp] ; Загружаем адрес таблицы IDT в специальный регистр процессора
    ret         ; Возвращаемся в Си-код


isr_common:
    pusha               ; Сохраняем регистры
    mov ax, ds
    push eax            ; Сохраняем сегмент данных

    mov ax, 0x10        ; Сегмент данных ядра
    mov ds, ax
    mov es, ax

    push esp            ; Передаем указатель на структуру registers_t
    call isr_handler_c
    add esp, 4          ; Очищаем аргумент функции

    pop eax
    mov ds, ax
    mov es, ax
    popa                ; Восстанавливаем регистры

    add esp, 8          ; Очищаем код ошибки и номер прерывания (ВАЖНО!)
    iret                ; Возврат из прерывания