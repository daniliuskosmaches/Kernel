//
// Created by nazar on 04.03.26.
//
#include "../include/gdt.h"
#include "../include/io.h"



static struct gdt_entry gdt[3];
static struct gdt_ptr   gdtp;

extern void gdt_flush(uint32_t);  // из gDtAsm.asm

static void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_install(void) {
    gdtp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdtp.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0,          0,    0);     // null
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code  CS=0x08
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data  DS=0x10

    gdt_flush((uint32_t)&gdtp);
}