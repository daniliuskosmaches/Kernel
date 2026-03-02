ENTRY(_start)

SECTIONS
{
    /* Загружаемся на 1МБ */
    . = 0x100000;

    /* Секция КОДА: Только чтение и выполнение */
    .text ALIGN(4K) :
    {
        *(.multiboot)
        *(.text)
    }

    /* Секция ДАННЫХ: Чтение и запись */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.rodata)
    }

    /* Секция BSS: Обнуленные данные */
    .bss ALIGN(4K) :
    {
        *(.common)
        *(.bss)
    }

    /* РАЗДЕЛИТЕЛЬ: Все, что после этой точки — свободная память для Slab и Бэкенда */
        . = ALIGN(4K);
        .slab ALIGN(4K) :
        {
            *(.slab)
        }
        .backend ALIGN(4K) :
        {
            *(.backend)
        }

    /* Добавляем символ конца ядра для отладки */
    _kernel_end = .;
}