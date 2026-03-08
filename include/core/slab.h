#ifndef SLAB_H
#define SLAB_H


#include "../lib/stdint.h"


#define SLAB_MAGIC 0xCAFEBABE

// Структура одного свободного объекта (наложенная на саму память объекта)
typedef struct slab_obj {
    struct slab_obj* next;
} slab_obj_t;

// Заголовок каждой 4KB страницы
typedef struct slab_page {
    uint32_t magic;
    uint32_t obj_size;
    uint32_t free_count;
    slab_obj_t* free_list;
    struct slab_page* next;
} slab_page_t;

// Управление кэшем конкретного типа объектов
typedef struct slab_cache {
    uint32_t obj_size;         // Размер одного объекта
    uint32_t objects_per_page; // Сколько их влезает в 4KB
    slab_page_t* full_list;    // Полные страницы
    slab_page_t* partial_list; // Частично заполненные
    slab_page_t* empty_list;   // Пустые (для резерва)
} slab_cache_t;

// Прототипы
void slab_init();
slab_cache_t* slab_cache_create(uint32_t object_size);
void* slab_alloc(slab_cache_t* cache);
void slab_free(slab_cache_t* cache, void* ptr);

// Быстрые аллокаторы для общих нужд (на базе кэшей 32, 64, 128, 256)
void* kmalloc_fast(uint32_t size);
void kfree_fast(void* ptr);

#endif
