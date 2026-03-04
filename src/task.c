#include "../include/core/task.h"
#include "../include/core/slab.h"
#include "../include/core/vmm.h"
#include "../include/pmm.h"
#include "string.h"
#include "../include/switch_to_task.h"
#include <stdint.h>


task_t *current_task = 0;
task_t *ready_queue = 0;
static uint32_t next_pid = 1;
task_t* task_create(void (*entry_point)(), const char* name) {
    task_t *new_task = (task_t*)kmalloc_fast(sizeof(task_t));
    memset(new_task, 0, sizeof(task_t));

    void* stack = pmm_alloc_page();
    // Считаем адрес верхушки как число, а не как указатель
    uint32_t stack_top = (uint32_t)stack + 4096;
    uint32_t context_ptr = stack_top - sizeof(context_t);

    context_t *ctx = (context_t*)context_ptr;
    memset(ctx, 0, sizeof(context_t));
    ctx->eip = (uint32_t)entry_point;
    ctx->ebp = 0;

    new_task->pid = next_pid++;
    new_task->esp = context_ptr; // Сохраняем точный адрес
    new_task->ebp = context_ptr;
    new_task->pg_dir_phys = vmm_get_cr3();
    strcpy(new_task->name, name);

    new_task->next = ready_queue;
    ready_queue = new_task;

    return new_task;
}

void task_init(void) {
    // Превращаем текущий поток выполнения (kmain) в задачу
    current_task = (task_t*)kmalloc_fast(sizeof(task_t));
    memset(current_task, 0, sizeof(task_t));

    current_task->pid = next_pid++;
    current_task->pg_dir_phys = vmm_get_cr3();
    current_task->next = 0;

    ready_queue = current_task;
}

void scheduler_run() {
    if (!ready_queue || !current_task) return;

    // Выбираем следующую задачу
    task_t *next = current_task->next;
    if (!next) next = ready_queue;

    if (next == current_task) return; // Если задача одна — ничего не делаем

    task_t *prev = current_task;
    current_task = next;

    // Вызываем переключатель. Убедись, что аргументы совпадают с switch_to_task.asm
    switch_to_task(next->eip, next->esp, next->ebp, next->pg_dir_phys);
}



char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}