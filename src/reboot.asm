[BITS 32]

; Reboot the system by writing to the keyboard controller's command port.
section .text
; Внешняя функция для перезагрузки системы
global _native_reboot
global native_reboot



native_reboot:
_native_reboot:
cli                    ; Отключаем прерывания
push dword 0
push dword 0
lidt [esp]              ; Загружаем пустой IDT, чтобы отключить обработку прерываний
int 3 ; Вызываем прерывание 3 (INT 3) для генерации исключения, которое должно перезагрузить систему
;на случай, если INT 3 не сработает, выполняем прямой доступ к контроллеру клавиатуры для перезагрузки

.loop:
  hlt
  jmp .loop

               ; Останавливаем
