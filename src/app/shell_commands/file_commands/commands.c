//
// Created by nazar on 08.03.26.
//

#include "../../../../include/vfs/vfs.h"
#include "../../../../include/core/slab.h"
#include "../../../../include/lib/string.h"
#include "../../../../include/vga.h"
#include "../../../../include/commands.h"



// Кэш для записей в директории (чтобы не фрагментировать память)
static slab_cache_t* dirent_cache= NULL;


// Глобальная переменная для шелла
extern vfs_node_t* current_dir;


vfs_node_t* ramfs_create(vfs_node_t* parent, char* name, uint32_t flags) {
    if (!parent) return NULL; // Защита от Page Fault

    vfs_node_t* node = vfs_create_node(name, flags);
    if (!node) return NULL;

    ramfs_entry_t* entry = (ramfs_entry_t*)slab_alloc(dirent_cache);
    if (!entry) return NULL; // Если память кончилась

    node->parent = parent; // Не забываем про обратную связь для 'at ..'

    strncpy(entry->name, name, 31);
    entry->node = node;

    entry->next = (ramfs_entry_t*)parent->priv_data;
    parent->priv_data = (void*)entry;

    return node;
}



void ramfs_init() {
    // Этот кэш нужен специально для записей в директориях
    dirent_cache = slab_cache_create(sizeof(ramfs_entry_t));
}

void ramfs_list(vfs_node_t* dir) {
    ramfs_entry_t* entry = (ramfs_entry_t*)dir->priv_data;

    if (!entry) {
        terminal_write_string("(empty)\n");
        return;
    }

    while (entry) {
        if (entry->node->flags & VFS_DIRECTORY) terminal_write_string("[D] ");
        else terminal_write_string("[F] ");

        terminal_write_string(entry->name);
        terminal_write_string("\n");
        entry = entry->next;
    }
}

vfs_node_t* ramfs_find(vfs_node_t* dir, char* name) {
    ramfs_entry_t* entry = (ramfs_entry_t*)dir->priv_data;

    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->node;
        }
        entry = entry->next;
    }
    return NULL; // Не нашли
}