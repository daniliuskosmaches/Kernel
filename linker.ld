/* linker.ld */

ENTRY(_start)

SECTIONS
{
    /* Загрузочный адрес ядра, 1 Мегабайт */
    . = 0x100000;

    .text :
    {
        /* Важно: Multiboot Header должен быть первым! */
        *(.multiboot)

        /* Весь остальной исполняемый код C и Ассемблера */
        *(.text)
    }

    .data ALIGN(0x1000) :
    {
        *(.data)
    }

    .bss ALIGN(0x1000) :
    {
        *(.bss)
    }
}