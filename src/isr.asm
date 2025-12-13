; src/isr.asm (ФУЛЛ-ФИКС: Векторы 0-47)

extern isr_handler_c

section .text
global isr_common

; -------------------------------------------
; Макросы для генерации исключений (ISR 0-31)
; -------------------------------------------

; Исключения, которые толкают код ошибки на стек (Error Code)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli                   ; Отключить прерывания
    push byte %1          ; Вектор прерывания (Код ошибки уже на стеке)
    jmp isr_common
%endmacro

; Исключения, которые НЕ толкают код ошибки (No Error Code)
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                   ; Отключить прерывания
    push byte 0           ; Эмулируем код ошибки (для симметрии стека)
    push byte %1          ; Вектор прерывания
    jmp isr_common
%endmacro

; Исключения с кодом ошибки: 8, 10, 11, 12, 13, 14, 17
ISR_ERRCODE 8
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_ERRCODE 17

; Исключения БЕЗ кода ошибки: 0-7, 9, 15, 16, 18-31
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_NOERRCODE 9
ISR_NOERRCODE 15
ISR_NOERRCODE 16
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


; -------------------------------------------
; Макросы для генерации прерываний (IRQ 0-15)
; -------------------------------------------
; IRQ 0-15 перемаплены на 0x20 - 0x2F
%macro IRQ 2
global irq%1
irq%1:
    cli                   ; Отключить прерывания
    push byte 0           ; Эмулируем код ошибки
    push byte %2          ; Вектор прерывания (0x20, 0x21, ...)
    jmp isr_common
%endmacro

IRQ 0, 0x20
IRQ 1, 0x21
IRQ 2, 0x22
IRQ 3, 0x23
IRQ 4, 0x24
IRQ 5, 0x25
IRQ 6, 0x26
IRQ 7, 0x27
IRQ 8, 0x28
IRQ 9, 0x29
IRQ 10, 0x2A
IRQ 11, 0x2B
IRQ 12, 0x2C
IRQ 13, 0x2D
IRQ 14, 0x2E
IRQ 15, 0x2F

; -------------------------------------------
; Общая логика обработки (C-Call)
; -------------------------------------------
isr_common:
    pusha                ; 1. Сохранить все регистры (EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX)

    ; 2. Сохранить старый сегмент данных DS и установить сегмент ядра (0x10)
    mov ax, ds
    push eax             ; Сохраняем DS на стеке
    mov ax, 0x10         ; Селектор сегмента данных ядра (обычно 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; СОХРАНЯЕМ int_no ДО ЛЮБЫХ ОПЕРАЦИЙ
    mov eax, [esp + 36]    ; int_no находится в [esp+36] (после pusha + ds)
    mov ebx, eax           ; EBX = int_no (сохраняем на случай)

    ; 3. Вызываем C-обработчик, передавая ему указатель на структуру registers_t (ESP)
    mov eax, esp
    push eax             ; Передаем указатель на регистры (EAX) как аргумент (regs*)
    push ebx             ; arg2: int_no

    call isr_handler_c   ; 4. Вызов обработчика на C

    add esp, 8             ; Очистить 2 аргумента

    pop eax              ; 5. Очистить стек от аргумента (EAX)

    ; 6. Восстановить старый сегмент данных
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                 ; 7. Восстановить регистры общего назначения

    ; ОТПРАВЛЯЕМ EOI на основе сохраненного int_no (в EBX)
    cmp ebx, 0x20          ; IRQ начинается с 0x20
    jl .no_eoi

    cmp ebx, 0x28
    jl .master_only

    ; Slave PIC
    mov al, 0x20
    out 0xA0, al

.master_only:
    mov al, 0x20
    out 0x20, al

.no_eoi:
    ; 8. Очистить стек от номера вектора и кода ошибки (2 x 4 байта = 8 байт)
    add esp, 8
    iret                 ; 9. Возврат из прерывания