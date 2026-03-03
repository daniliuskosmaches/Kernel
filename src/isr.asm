; src/isr.asm - ИСПРАВЛЕННАЯ ВЕРСИЯ
extern isr_handler_c

section .text
global isr_common

; --- Макросы ---

; ИСПРАВЛЕНО: ISR_ERRCODE - CPU уже положил err_code, мы только пушим int_no
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    ; err_code уже на стеке от CPU
    push dword %1       ; FIX: push dword вместо push byte — гарантирует 4 байта
    jmp isr_common
%endmacro

; ISR_NOERRCODE - CPU не кладёт err_code, эмулируем 0
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push dword 0        ; FIX: push dword — заглушка err_code (4 байта)
    push dword %1       ; FIX: push dword — номер прерывания (4 байта)
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

ISR_NOERRCODE 128  ; Системный вызов INT 0x80

; --- Аппаратные прерывания (IRQ 0-15 -> INT 32-47) ---

%macro IRQ 2
global irq%1
irq%1:
    push dword 0        ; FIX: push dword
    push dword %2       ; FIX: push dword — номер вектора (32-47)
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
extern idtp

idt_load:
    lidt [idtp]
    ret


isr_common:
    pusha               ; Сохраняем: eax, ecx, edx, ebx, esp, ebp, esi, edi
    mov ax, ds
    push eax            ; Сохраняем ds

    mov ax, 0x10        ; Сегмент данных ядра
    mov ds, ax
    mov es, ax
    mov fs, ax          ; FIX: также обновляем fs и gs
    mov gs, ax

    push esp            ; Передаём указатель на registers_t в isr_handler_c
    call isr_handler_c
    add esp, 4          ; Убираем аргумент

    pop eax             ; Восстанавливаем ds
    mov ds, ax
    mov es, ax
    mov fs, ax          ; FIX: восстанавливаем fs и gs
    mov gs, ax

    popa                ; Восстанавливаем регистры
    add esp, 8          ; Убираем err_code и int_no
    iret                ; Возврат из прерывания