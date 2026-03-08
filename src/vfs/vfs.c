//
// Created by nazar on 08.03.26.
//
#include "../../include/vfs/vfs.h"

#include "../../include/core/slab.h"
#include "../../include/lib/stddef.h"
#include "../../include/lib/string.h"
#include "../../include/commands.h"

// Кэш для быстрого выделения узлов VFS
static slab_cache_t* vnode_cache;
vfs_node_t* vfs_root = 0;

vfs_node_t* current_dir = 0;


void vfs_init() {
    vnode_cache = slab_cache_create(sizeof(vfs_node_t));

    // ВАЖНО: Создаем корень сразу при старте VFS
    vfs_root = vfs_create_node("/", VFS_DIRECTORY);
    vfs_root->parent = vfs_root; // У корня родитель — он сам
    vfs_root->priv_data = NULL;

    // Здесь позже будет монтирование корневой ФС (например, initrd)
}

vfs_node_t* vfs_find_in_dir(vfs_node_t* dir, char* name) {
    if (!dir || !(dir->flags & VFS_DIRECTORY)) {
        return NULL;
    }

    // Если это наша оперативная ФС, вызываем её поиск
    return ramfs_find(dir, name);
}


vfs_node_t* vfs_create_node(char* name, uint32_t flags) {
    vfs_node_t* node = (vfs_node_t*)slab_alloc(vnode_cache);
    if (!node) return NULL;

    memset(node, 0, sizeof(vfs_node_t)); // Обнулит и parent, и priv_data
    strncpy(node->name, name, 31);
    node->flags = flags;
    node->parent = NULL; // На всякий случай явно

    return node;
}

uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Если у узла есть своя функция записи (которую проставит Ramfs)
    if (node && node->write) {
        return node->write(node, offset, size, buffer);
    }
    return 0; // Запись не поддерживается
}

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node && node->read) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}



// Открытие файла (проход по дереву)
vfs_node_t* vfs_open(const char* path) {
    // В полноценной реализации здесь нужно парсить строку "/" -> "bin" -> "sh"
    // Для начала вернем корень, если путь "/"
    if (strcmp(path, "/") == 0) {
        return vfs_root;
    }

    // Логика обхода дерева будет тут
    return NULL;
}

