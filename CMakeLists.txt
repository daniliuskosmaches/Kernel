cmake_minimum_required(VERSION 4.0)
project(untitled2 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled2 main.c
        src/kernel.c
        src/system.h
        src/idt.c
        src/vga.c
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
        src/string.c
        src/kheap.h
        src/keyboard.h
        src/keyboard.c
        src/io.h
        src/isr.h
        src/timer.h


)
