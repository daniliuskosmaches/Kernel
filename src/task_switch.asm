global switch_to_task
switch_to_task:
    ; Аргументы: [esp+4]=eip, [esp+8]=esp, [esp+12]=ebp, [esp+16]=cr3

    ; 1. Сохраняем контекст текущей задачи не нужно, так как мы
    ; просто меняем указатели. Но нужно загрузить новые:
    mov ecx, [esp + 4]  ; EIP
    mov edx, [esp + 8]  ; ESP
    mov ebx, [esp + 12] ; EBP
    mov eax, [esp + 16] ; CR3 (Page Directory)

    ; 2. Переключаем страницу памяти (если нужно)
    mov cr3, eax

    ; 3. Устанавливаем новые указатели стека
    mov esp, edx
    mov ebp, ebx

    ; 4. Прыгаем в новую задачу
    jmp ecx