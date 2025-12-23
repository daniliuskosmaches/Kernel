; src/entry.asm (С КОРРЕКТНЫМ MULTIBOOT HEADER)

; -------------------------------------------
; 1. Multiboot Header (Обязательно для GRUB)
; -------------------------------------------
section .multiboot
MBOOT_PAGE_ALIGN    equ 1 << 0  ; Выровнять по странице
MBOOT_MEM_INFO      equ 1 << 1  ; Передать инфо о памяти
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

dd MBOOT_HEADER_MAGIC
dd MBOOT_HEADER_FLAGS
dd MBOOT_CHECKSUM

; -------------------------------------------
; 2. Точка Входа Ядра
; -------------------------------------------
section .text
global _start
extern kmain
global vmm_get_cr3


_start:
    ; 1. Настройка Стека:
    mov esp, stack_top

    ; 2. Передача аргументов в kmain:
    push ebx    ; push mbi
    push eax    ; push multiboot_magic

    ; 3. Вызов главной функции ядра на C
    call kmain

    ; 4. Очистка стека от аргументов
    add esp, 8

    ; 5. Бесконечный цикл
    cli
.loop:
    hlt
    jmp .loop

; --- Данные ---



section .bss
resb 8192
stack_top:


