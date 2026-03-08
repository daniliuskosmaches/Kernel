cmake_minimum_required(VERSION 4.0)
project(untitled2 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled2 main.c
        src/kernel.c
        src/system.h
        src/idt.c
        src/app/visual_graphic_application/vga.c
        src/timer.c
        src/pmm.c
        src/vmm.c
        src/vga.h
        src/pmm.h
        src/string.h
        src/stdint.h
        src/stddef.h
        src/task.h
        src/task.c
        src/kernel.h
        src/kheap.c
        src/lib/string.c
        src/kheap.h
        src/keyboard.h
        src/keyboard.c
        src/io.h
        src/isr.h
        src/timer.h
        src/shell.h
        src/app/shell.c
        src/syscall.c
        src/syscall.h
        src/slab.c
        include/core/slab.h
        src/app/shell.c
        src/kheap.c
        include/switch_to_task.h
        include/gdt.h
        src/gdt.c
        src/app/shell_commands/shell_commands.c
        include/app/shell_commands.h
        include/lib/stdio.h
        src/app/text_editor/piko.c
        src/vfs/vfs.c
        include/vfs/vfs.h
        include/app/text_editor/piko.h
        include/types.h
        src/app/shell_commands/file_commands/commands.c
        include/commands.h


)
