#include "../include/pmm.h"
#include "../include/multiboot.h"
#include "../include/vga.h"
#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_PHYS_MEM 0x10000000 // 256 MB
#define MAX_PAGES (MAX_PHYS_MEM / PAGE_SIZE)

extern uint32_t placement_address;

unsigned int *pmm_bitmap;
unsigned int total_pages = 0;
unsigned int used_pages = 0;

void set_frame(unsigned int frame_index) {
    pmm_bitmap[frame_index / 32] |= (1 << (frame_index % 32));
}

void clear_frame(unsigned int frame_index) {
    pmm_bitmap[frame_index / 32] &= ~(1 << (frame_index % 32));
}

int test_frame(unsigned int frame_index) {
    return pmm_bitmap[frame_index / 32] & (1 << (frame_index % 32));
}

void pmm_init(multiboot_info_t *mbi) {
    uint32_t end_of_kernel = (uint32_t)&placement_address;
    if (end_of_kernel & 0xFFF) end_of_kernel = (end_of_kernel & 0xFFFFF000) + 0x1000;

    pmm_bitmap = (unsigned int *)end_of_kernel;
    uint32_t bitmap_size = MAX_PAGES / 8;
    uint32_t pmm_end = (uint32_t)pmm_bitmap + bitmap_size;
    if (pmm_end & 0xFFF) pmm_end = (pmm_end & 0xFFFFF000) + 0x1000;

    total_pages = MAX_PAGES;
    used_pages = MAX_PAGES;

    // Заполняем всё единицами (занято)
    for (uint32_t i = 0; i < (MAX_PAGES / 32); i++) pmm_bitmap[i] = 0xFFFFFFFF;

    // Читаем карту памяти
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
    multiboot_memory_map_t *mmap_end = (multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);

    while (mmap < mmap_end) {
        if (mmap->type == 1) {
            uint32_t block_start = (uint32_t)mmap->addr;
            uint32_t block_end = block_start + (uint32_t)mmap->len;
            if (block_start & 0xFFF) block_start = (block_start & 0xFFFFF000) + 0x1000;
            block_end &= 0xFFFFF000;

            for (uint32_t addr = block_start; addr < block_end && addr < MAX_PHYS_MEM; addr += PAGE_SIZE) {
                if (test_frame(addr / PAGE_SIZE)) {
                    clear_frame(addr / PAGE_SIZE);
                    used_pages--;
                }
            }
        }
        mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }

    // Резервируем ядро и битмап обратно
    for (uint32_t addr = 0; addr < pmm_end; addr += PAGE_SIZE) {
        if (test_frame(addr / PAGE_SIZE) == 0) {
            set_frame(addr / PAGE_SIZE);
            used_pages++;
        }
    }

    terminal_write_string("PMM: OK. Free frames: ");
    terminal_write_hex(total_pages - used_pages);
    terminal_write_string("\n");
}

void *pmm_alloc_page(void) {
    for (unsigned int i = 0; i < MAX_PAGES; i++) {
        if (test_frame(i) == 0) {
            set_frame(i);
            used_pages++;
            return (void *)(i * PAGE_SIZE);
        }
    }
    return 0;
}