//
// Created by nazar on 09.12.25.
//
// src/task.h

// src/task.h

#ifndef TASK_H
#define TASK_H

#include "stdint.h"
#include "../idt.h"

// Определение структуры задачи (Process Control Block)
typedef struct task {
    uint32_t esp;        // 1. Указатель стека (должен быть первым для удобства ASM)
    uint32_t ebp;        // 2. Базовый указатель
    uint32_t eip;        // 3. Указатель инструкции

    // 4. Физический адрес Page Directory (для переключения памяти)
    uint32_t pg_dir_phys;

    // Метаданные
    uint32_t pid;         // ID процесса
    // Добавьте указатель на следующую задачу для создания списка
    struct task *next;

} task_t;

// Глобальное объявление текущей задачи и списка задач
extern task_t *current_task; // Указатель на текущую выполняемую задачу
extern task_t *ready_queue;  // Указатель на начало списка готовых задач

// Прототипы функций
void task_init(void);
void switch_task(void);
task_t *fork(void); // Функция для создания нового процесса (позже)

#endif