#include "../include/core/task.h"
#include "../include/core/slab.h"
#include "../include/core/vmm.h"
#include "../include/lib/string.h"

task_t *current_task = 0;
task_t *ready_queue = 0;
static uint32_t next_pid = 1;

// Создание новой легкой задачи (Fiber)
task_t* task_create(void (*entry_point)(), const char* name) {
    // Используем твой новый скоростной аллокатор!
    task_t *new_task = (task_t*)kmalloc_fast(sizeof(task_t));
    memset(new_task, 0, sizeof(task_t));

    // Выделяем стек для задачи (4KB)
    void* stack = pmm_alloc_page();
    uint32_t* esp = (uint32_t*)((uint32_t)stack + 4096);

    // Подготавливаем стек, чтобы switch_to_task мог "вернуться" в entry_point
    *(--esp) = (uint32_t)entry_point; // Куда прыгнуть
    *(--esp) = 0;                    // Фиктивный EBP

    new_task->pid = next_pid++;
    new_task->esp = (uint32_t)esp;
    new_task->ebp = (uint32_t)esp;
    new_task->pg_dir_phys = vmm_get_cr3(); // Identity mapping
    strcpy(new_task->name, name);

    // Добавляем в очередь
    new_task->next = ready_queue;
    ready_queue = new_task;

    return new_task;
}

void task_init(void) {
    // Превращаем kmain в первую задачу
    current_task = (task_t*)kmalloc_fast(sizeof(task_t));
    current_task->pid = next_pid++;
    current_task->pg_dir_phys = vmm_get_cr3();
    current_task->next = 0;
    ready_queue = current_task;
}

void scheduler_run() {
    if (!current_task->next && current_task == ready_queue) return;

    // Выбираем следующую задачу (Round Robin)
    task_t *next = current_task->next;
    if (!next) next = ready_queue;

    task_t *prev = current_task;
    current_task = next;

    // Вызываем твой ассемблерный переключатель
    switch_to_task(next->eip, next->esp, next->ebp, next->pg_dir_phys);
}