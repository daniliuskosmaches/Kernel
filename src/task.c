//
// Created by nazar on 09.12.25.
//
// src/task.c
// src/task.c

#include "task.h"
#include "kernel.h"
#include "string.h"
#include "vmm.h"


extern void switch_to_task(uint32_t eip, uint32_t esp, uint32_t ebp, uint32_t pg_dir);
// Глобальные переменные определяются здесь
task_t *current_task = 0;
task_t *ready_queue = 0;  // Очередь задач

// Временный счетчик ID
static uint32_t next_pid = 1;

// -----------------------------------------------------------------
// Инициализация подсистемы многозадачности
// -----------------------------------------------------------------

void task_init(void) {
    current_task = (task_t*)kmalloc(sizeof(task_t));
    memset(current_task, 0, sizeof(task_t));

    current_task->pid = next_pid++;

    // Получаем РЕАЛЬНЫЕ значения регистров текущего стека
    uint32_t esp, ebp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    __asm__ volatile("mov %%ebp, %0" : "=r"(ebp));

    current_task->esp = esp;
    current_task->ebp = ebp;
    current_task->pg_dir_phys = vmm_get_cr3(); // Получаем текущий CR3
    current_task->next = 0;

    ready_queue = current_task;
}

// -----------------------------------------------------------------
// Переключение задач (Context Switch)
// -----------------------------------------------------------------

// Объявляется как extern, так как основная работа будет в ASM
extern void switch_to_task(uint32_t eip, uint32_t esp, uint32_t ebp, uint32_t pg_dir);

// Функция планировщика (вызывается из timer_handler)
void switch_task(void) {
    // 1. Если нет задач, нечего делать
    if (!current_task || !current_task->next) {
        return;
    }

    // 2. Сохраняем контекст текущей задачи
    // (Этот шаг будет выполнен в ASM, который вызовет switch_task и сохранит regs)

    // 3. Выбираем следующую задачу (простой Round-Robin)
    current_task = current_task->next;

    // 4. Восстанавливаем контекст следующей задачи
    uint32_t eip = current_task->eip;
    uint32_t esp = current_task->esp;
    uint32_t ebp = current_task->ebp;
    uint32_t pg_dir = current_task->pg_dir_phys;

    // 5. Переключаемся!
    // ЭТО ОЧЕНЬ СЛОЖНЫЙ ШАГ, который требует ASM-кода.
    // Пока что оставим здесь вызов:
    // switch_to_task(eip, esp, ebp, pg_dir);
    switch_to_task(eip, esp, ebp, pg_dir);
}

task_t *fork(void) {
    // 1. Выделяем память для новой структуры задачи (PCB)
    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));

    // 2. Копируем текущий контекст (контекст родителя)
    // Копирование ВСЕХ полей, включая ESP, EBP, EIP
    memcpy(new_task, current_task, sizeof(task_t));

    // 3. Устанавливаем новый PID
    new_task->pid = next_pid++;

    // 4. Клонирование виртуального адресного пространства
    // Этот шаг требует функции клонирования Page Directory (из VMM)
    // Мы должны создать новый Page Directory, но скопировать ВСЕ входы из старого.
    // Если у вас нет этой функции, то пока просто используем каталог ядра:
    // new_task->pg_dir_phys = vmm_get_cr3(); // (Временное решение)

    // 5. Создание нового стека (ОЧЕНЬ ВАЖНО!)
    // В идеале мы должны выделить новый стек (4KB), скопировать старый стек,
    // и скорректировать esp/ebp.

    // Для минимального работающего примера:
    new_task->next = current_task->next;
    current_task->next = new_task;

    // 6. Коррекция EIP/ESP (КЛЮЧЕВОЙ МОМЕНТ)
    // Когда планировщик переключится на новую задачу:
    // a) Родитель (текущая задача) получит управление здесь и вернет new_task (т.е. PID ребенка).
    // b) Ребенок получит управление, когда переключится контекст, и он должен начать
    //    с какого-то адреса, который будет считаться "началом" его выполнения после fork.

    // *** Пока оставим ESP/EBP/EIP скопированными. Реальное клонирование стека - сложный шаг. ***

    return new_task;
}