//
// Created by nazar on 08.03.26.
//

#ifndef UNTITLED2_VFS_H
#define UNTITLED2_VFS_H



#ifndef VFS_H
#define VFS_H

#include <stdint.h>

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHARDEVICE  0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE        0x05

struct vfs_node;

// Типы функций, которые должна реализовать каждая конкретная ФС
typedef uint32_t (*read_type_t)(struct vfs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct vfs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(struct vfs_node*);
typedef void (*close_type_t)(struct vfs_node*);
typedef struct vfs_node* (*finddir_type_t)(struct vfs_node*, char *name);

// Главная структура узла файловой системы (vnode)
typedef struct vfs_node {
    char name[128];     // Имя файла
    uint32_t mask;      // Права доступа
    uint32_t uid;       // ID пользователя
    uint32_t gid;       // ID группы
    uint32_t flags;     // Тип (файл, папка и т.д.)
    uint32_t inode;     // Номер инода (зависит от конкретной ФС)
    uint32_t length;    // Размер файла в байтах
    uint32_t impl;      // Вспомогательное поле для реализации конкретной ФС

    // Указатели на функции (интерфейс)
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    finddir_type_t finddir;
    void* priv_data;

    struct vfs_node* parent;




    struct vfs_node *ptr; // Используется для монтирования или ссылок
} vfs_node_t;

// Глобальный корень системы
extern vfs_node_t *vfs_root;
extern vfs_node_t* current_dir;

// Прототипы функций VFS
void vfs_init();
uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
vfs_node_t *vfs_open(const char *path);
vfs_node_t* vfs_create_node(char* name, uint32_t flags);
vfs_node_t* vfs_find_in_dir(vfs_node_t* dir, char* name);

#endif
#endif //UNTITLED2_VFS_H