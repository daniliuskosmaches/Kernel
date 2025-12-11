; src/isr.asm
; src/isr.asm (ИСПРАВЛЕННАЯ ЛОГИКА C-Call)

extern isr_handler_c
section .text
global isr_irq0
global isr_irq1
global isr_handler_c
global isr_common


; -------------------------------------------
; Обертки для таймера (IRQ 0) и клавиатуры (IRQ 1)
; -------------------------------------------
isr_irq0:
    push byte 0          ; Эмулируем код ошибки
    push byte 0x20       ; Вектор прерывания (0x20)
    jmp isr_common       ; Перейти к C-обработчику

; --- Обработчик IRQ 1 (Клавиатура) ---
isr_irq1:
    push byte 0          ; Эмулируем код ошибки
    push byte 0x21       ; Вектор прерывания (0x21)
    jmp isr_common

; -------------------------------------------
; Общая логика обработки (C-Call)
; -------------------------------------------
isr_common:
    pusha                ; 1. Сохранить все регистры (EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX)

    ; 2. Сохранить старый сегмент данных DS и установить сегмент ядра (0x10)
    mov ax, ds
    push eax
    mov ax, 0x10         ; Селектор сегмента данных ядра
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 3. Вызываем C-обработчик, передавая ему указатель на структуру registers_t (ESP)
    mov eax, esp
    push eax             ; Передаем указатель на регистры (EAX) как аргумент (regs*)

    call isr_handler_c   ; 4. Вызов обработчика на C

    pop eax              ; 5. Очистить стек от аргумента (EAX)

    ; 6. Восстановить старый сегмент данных
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                 ; 7. Восстановить все регистры

    ; 8. Очистить стек от номера вектора и кода ошибки
    add esp, 8
    iret                 ; 9. Возврат из прерывания