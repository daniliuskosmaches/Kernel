#include "string.h"
#include "vmm.h"
#include "pmm.h"
#include "vga.h"

page_directory_t *current_page_directory = 0;

void vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    virt_addr &= 0xFFFFF000;
    phys_addr &= 0xFFFFF000;

    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    page_directory_t *pd = current_page_directory;

    // 1. Проверяем наличие таблицы страниц
    if (!(pd->entries[pd_index].present)) {
        uint32_t new_pt_phys = (uint32_t)pmm_alloc_page();
        if (new_pt_phys == 0) return; // Функция void, просто выходим

        // Используем table_addr, как в твоем vmm.h
        pd->entries[pd_index].table_addr = new_pt_phys >> 12;
        pd->entries[pd_index].present = 1;
        pd->entries[pd_index].rw = 1;

        // Очищаем новую таблицу (адрес тот же, так как у нас Identity Mapping)
        memset((void*)(new_pt_phys), 0, PAGE_SIZE);
    }

    // 2. Получаем указатель на таблицу страниц
    // Достаем адрес из table_addr
    page_table_t *pt = (page_table_t*)(pd->entries[pd_index].table_addr << 12);

    // 3. Мапим страницу. Используем entries и frame_addr из vmm.h
    pt->entries[pt_index].frame_addr = phys_addr >> 12;
    pt->entries[pt_index].present = 1;
    pt->entries[pt_index].rw = (flags & PAGE_RW) ? 1 : 0;
    pt->entries[pt_index].user = (flags & PAGE_USER) ? 1 : 0;

    // Сброс TLB (чтобы процессор забыл старый адрес)
    __asm__ __volatile__("invlpg (%0)" ::"r" (virt_addr));
}

void vmm_init(void) {
    terminal_write_string("  -> Allocating Page Directory... ");
    page_directory_t *new_pd = (page_directory_t*)pmm_alloc_page();
    if (new_pd == 0) {
        for(;;) __asm__ volatile("cli; hlt");
    }
    terminal_write_string("OK\n");

    memset(new_pd, 0, PAGE_SIZE);
    current_page_directory = new_pd;

    terminal_write_string("  -> Identity mapping first 16MB... ");
    uint32_t identity_size = 0x1000000;
    uint32_t pages_mapped = 0;

    for (uint32_t addr = 0; addr < identity_size; addr += PAGE_SIZE) {
        vmm_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
        pages_mapped++;
    }

    terminal_write_string("DONE\n");

    vmm_switch_directory(new_pd);
}

void vmm_switch_directory(page_directory_t *dir) {
    uint32_t pd_phys_addr = (uint32_t)dir;

    __asm__ __volatile__ (
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0"
        : : "r"(pd_phys_addr) : "eax"
    );
}
uint32_t vmm_get_cr3(void) {
    uint32_t cr3;
    __asm__ __volatile__ (
        "mov %%cr3, %0"
        : "=r"(cr3)     // Выходной параметр: записываем значение из CR3 в переменную cr3
    );
    return cr3;
}