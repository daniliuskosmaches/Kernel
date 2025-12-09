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


)
