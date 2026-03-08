//
// Created by nazar on 08.03.26.
//

#ifndef UNTITLED2_COMMANDS_H
#define UNTITLED2_COMMANDS_H


#ifndef RAMFS_H
#define RAMFS_H

#include "vfs/vfs.h"

// Структура записи в директории Ramfs
// Она связывает имя в файловой системе с объектом vfs_node_t
typedef struct ramfs_entry {
    char name[32];               // Имя конкретно в этой директории
    vfs_node_t* node;            // Указатель на сам узел (файл/папка)
    struct ramfs_entry* next;
    // Следующий элемент в списке (сосед по папке)
} ramfs_entry_t;

// Инициализация Ramfs (создание кэша Slab для записей)
void ramfs_init();

// Создание нового файла или папки внутри parent
vfs_node_t* ramfs_create(vfs_node_t* parent, char* name, uint32_t flags);

// Вывод списка файлов (для команды 'sh')
void ramfs_list(vfs_node_t* dir);

// Поиск узла по имени (для команды 'at')
vfs_node_t* ramfs_find(vfs_node_t* dir, char* name);

// Стандартные операции для VFS (назначим их в node->read / node->write)
uint32_t ramfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t ramfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);

#endif

#endif //UNTITLED2_COMMANDS_H