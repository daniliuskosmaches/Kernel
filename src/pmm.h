//
// Created by nazar on 09.12.25.
//
// src/pmm.h
#ifndef PMM_H
#define PMM_H

#include <stdint.h>

// Функции для управления физической памятью
void pmm_init(void *multiboot_info);
void *pmm_alloc_page(void);
void pmm_free_page(void *p);

// Дополнительные служебные функции
void pmm_mark_used(uint32_t addr, uint32_t len);
void pmm_mark_free(uint32_t addr, uint32_t len);

#endif