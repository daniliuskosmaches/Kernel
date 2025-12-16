//
// Created by nazar on 09.12.25.
//
// src/multiboot.h

#ifndef UNTITLED2_MULTIBOOT_H // <-- НАЧАЛО ЗАЩИТЫ
#define UNTITLED2_MULTIBOOT_H

// Структура для записи в таблице карты памяти
struct multiboot_mmap_entry {
    unsigned int size;
    unsigned long long addr;  // 64-битный адрес
    unsigned long long len;   // 64-битная длина
    unsigned int type;        // Тип: 1 = Доступно (Available)
} __attribute__((packed));

// Главная информационная структура
typedef struct {
    unsigned int flags;       // Флаги (важен бит 6 для карты памяти)

    unsigned int mmap_length; // Общая длина карты памяти
    unsigned int mmap_addr;   // Адрес начала списка записей
} multiboot_info_t;

// Магическое число Multiboot
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002
#define MULTIBOOT_FLAG_MMAP 0x00000040 // Флаг, указывающий на наличие карты памяти (бит 6)


#endif //UNTITLED2_MULTIBOOT_H <-- КОНЕЦ ЗАЩИТЫ