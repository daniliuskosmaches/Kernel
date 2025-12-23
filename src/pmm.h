#ifndef PMM_H
#define PMM_H


#include "multiboot.h"

// Функции для управления физической памятью
void pmm_init(multiboot_info_t *mbi);
void *pmm_alloc_page(void);
void pmm_free_page(void *p);


void pmm_mark_used(uint32_t addr, uint32_t len);
void pmm_mark_free(uint32_t addr, uint32_t len);





#endif
