#include "../include/lib/string.h"
#include "../include/core/vmm.h"
#include "../include/pmm.h"
#include "../include/vga.h"


page_directory_t *current_page_directory = 0;

void vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    page_directory_t *pd = current_page_directory;

    if (!(pd->entries[pd_index].present)) {
        uint32_t new_pt_phys = (uint32_t)pmm_alloc_page();
        if (new_pt_phys == 0) return;

        pd->entries[pd_index].table_addr = new_pt_phys >> 12;
        pd->entries[pd_index].present = 1;
        pd->entries[pd_index].rw = 1;

        // Это безопасно, так как вся память замаплена 1-в-1 в vmm_init
        memset((void*)(new_pt_phys), 0, 4096);
    }

    page_table_t *pt = (page_table_t*)(pd->entries[pd_index].table_addr << 12);
    pt->entries[pt_index].frame_addr = phys_addr >> 12;
    pt->entries[pt_index].present = 1;
    pt->entries[pt_index].rw = (flags & PAGE_RW) ? 1 : 0;
    pt->entries[pt_index].user = (flags & PAGE_USER) ? 1 : 0;

    __asm__ __volatile__("invlpg (%0)" ::"r" (virt_addr));
}

void vmm_init(void) {
    page_directory_t *new_pd = (page_directory_t*)pmm_alloc_page();
    memset(new_pd, 0, 4096);
    current_page_directory = new_pd;

    // Мапим ВСЮ физическую память (256МБ) 1-в-1
    uint32_t identity_size = 0x10000000;
    for (uint32_t addr = 0; addr < identity_size; addr += 4096) {
        vmm_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }

    vmm_switch_directory(new_pd);
    terminal_write_string("VMM: OK (256MB mapped)\n");
}

void vmm_switch_directory(page_directory_t *dir) {
    __asm__ __volatile__ (
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0"
        : : "r"(dir) : "eax"
    );
}


uint32_t vmm_get_cr3(void) {
    uint32_t cr3;
    __asm__ __volatile__ (
        "mov %%cr3, %0"
        : "=r"(cr3)
    );
    return cr3;
}