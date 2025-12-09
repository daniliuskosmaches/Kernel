; src/entry.asm

section .multiboot

; Определяем константы для Multiboot 1
MULTIBOOT_MAGIC equ 0x1BADB002
MULTIBOOT_FLAGS equ 0x00 ; Минимальные флаги
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

section .text
global _start
extern kmain  ; Объявляем, что kmain() будет функцией на C

_start:
    ; 1. Настройка Стека
    ; Мы устанавливаем указатель стека (ESP) на конец зарезервированного нами блока
    mov esp, stack_top

    ; 2. Вызов главной функции ядра на C
    call kmain

    ; 3. Бесконечный цикл после завершения работы ядра (на случай, если kmain() вернет управление)
    cli         ; Отключить прерывания
.loop:
    hlt         ; Остановить процессор
    jmp .loop

; --- Данные ---

section .bss
; Зарезервировать 8 КБ для стека
resb 8192
stack_top: