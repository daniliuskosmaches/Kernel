//
// Created by nazar on 03.01.26.
//

#include "pmm.h"
#include "vmm.h"
#include "string.h"
#include "vga.h"

#define SLAB_MAGIC 0x51AB51AB

// Структура одного свободного объекта в слабе
typedef struct slab_obj {
    struct slab_obj* next;
} slab_obj_t;

// Заголовок страницы слаба (размещается в начале каждой 4KB страницы)
typedef struct slab_page {
    uint32_t magic;
    uint32_t obj_size;
    uint32_t free_count;
    slab_obj_t* free_list;
    struct slab_page* next;
} slab_page_t;

// Основная структура кэша для конкретного размера объекта
typedef struct {
    uint32_t obj_size;
    slab_page_t* full_list;    // Список полностью занятых страниц
    slab_page_t* partial_list; // Страницы, где есть свободные места
} slab_cache_t;

// Определим кэши для самых частых размеров в бэкенде
// 32 и 64 — для сетевых заголовков, 128 и 256 — для структур задач/файберов
static slab_cache_t caches[4];

// Инициализация кэша
void slab_init_cache(slab_cache_t* cache, uint32_t size) {
    cache->obj_size = size;
    cache->full_list = NULL;
    cache->partial_list = NULL;
}

// Создание новой страницы слаба
static slab_page_t* slab_create_page(uint32_t obj_size) {
    // Берем физическую страницу у твоего PMM
    void* page_addr = pmm_alloc_page();
    if (!page_addr) return NULL;

    // В Unikernel у нас Identity Mapping, поэтому физ. адрес = вирт. адресу
    slab_page_t* page = (slab_page_t*)page_addr;
    page->magic = SLAB_MAGIC;
    page->obj_size = obj_size;
    page->next = NULL;

    // Вычисляем, сколько объектов влезет в страницу за вычетом заголовка
    uint32_t offset = sizeof(slab_page_t);
    page->free_list = (slab_obj_t*)((uint8_t*)page_addr + offset);
    page->free_count = (4096 - offset) / obj_size;

    // Нарезаем страницу на связный список объектов
    slab_obj_t* curr = page->free_list;
    for (uint32_t i = 0; i < page->free_count - 1; i++) {
        curr->next = (slab_obj_t*)((uint8_t*)curr + obj_size);
        curr = curr->next;
    }
    curr->next = NULL; // Последний объект

    return page;
}

void slab_init() {
    slab_init_cache(&caches[0], 32);
    slab_init_cache(&caches[1], 64);
    slab_init_cache(&caches[2], 128);
    slab_init_cache(&caches[3], 256);
    terminal_write_string("Slab Allocator: Initialized (32, 64, 128, 256 bytes buffers)\n");
}

void* kmalloc_fast(uint32_t size) {
    slab_cache_t* cache = NULL;

    // Выбираем подходящий кэш
    if (size <= 32) cache = &caches[0];
    else if (size <= 64) cache = &caches[1];
    else if (size <= 128) cache = &caches[2];
    else if (size <= 256) cache = &caches[3];
    else {
        // Если объект слишком большой, используем твой старый kheap
        // Или напрямую pmm_alloc_page, если это для сетевого буфера
        return pmm_alloc_page();
    }

    // Если в частичных страницах нет места, создаем новую страницу
    if (!cache->partial_list) {
        slab_page_t* new_page = slab_create_page(cache->obj_size);
        new_page->next = cache->partial_list;
        cache->partial_list = new_page;
    }

    slab_page_t* page = cache->partial_list;
    slab_obj_t* obj = page->free_list;

    page->free_list = obj->next;
    page->free_count--;

    // Если страница заполнилась, переносим её в full_list
    if (page->free_count == 0) {
        cache->partial_list = page->next;
        page->next = cache->full_list;
        cache->full_list = page;
    }

    return (void*)obj;
}

void kfree_fast(void* ptr) {
    if (!ptr) return;

    // Находим начало страницы (выравнивание по 4KB)
    slab_page_t* page = (slab_page_t*)((uint32_t)ptr & 0xFFFFF000);

    if (page->magic != SLAB_MAGIC) {
        // Это не наш объект, возможно он был выделен через PMM напрямую
        // pmm_free_page(ptr);
        return;
    }

    // Возвращаем объект в список свободных
    slab_obj_t* obj = (slab_obj_t*)ptr;
    obj->next = page->free_list;
    page->free_list = obj;
    page->free_count++;

    // Тут можно добавить логику переноса из full_list обратно в partial_list
}