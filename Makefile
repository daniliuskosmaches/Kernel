# Makefile (ФИНАЛЬНАЯ ИСПРАВЛЕННАЯ ВЕРСИЯ)

# --- Настройка Инструментов ---
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
QEMU = qemu-system-i386

# --- Файлы и Пути ---
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -nostdinc -I.
OBJECTS      = src/entry.o src/kernel.o src/asm_io.o src/idt.o src/isr.o src/vga.o src/timer.o src/pmm.o src/vmm.o src/task.o src/kheap.o src/string.o src/task_switch.o src/shell.o src/keyboard.o
LINKER_SCRIPT = linker.ld
KERNEL_ELF   = kernel.elf
KERNEL_BIN   = kernel.bin
GRUB_CFG     = grub.cfg  # КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Это исходный файл, а не путь назначения!

# --- Главные Цели ---

.PHONY: all iso run clean debug-wait gdb

all: $(KERNEL_BIN) iso

# --- Правила Компиляции ---

src/task_switch.o: src/task_switch.asm
	nasm -f elf src/task_switch.asm -o src/task_switch.o

src/kheap.o: src/kheap.c
	$(CC) $(CFLAGS) -c src/kheap.c -o src/kheap.o

# Линковка ELF -> BIN
$(KERNEL_BIN): $(OBJECTS) $(LINKER_SCRIPT)
	@echo "-> Линковка ELF: Создание $(KERNEL_ELF)"
	$(LD) -T $(LINKER_SCRIPT) -o $(KERNEL_ELF) $(OBJECTS)

	@echo "-> Создание BIN: $@"
	i686-elf-objcopy -O binary $(KERNEL_ELF) $(KERNEL_BIN)

# Одно общее правило для всех C файлов
src/%.o: src/%.c
	@echo "-> Компиляция C: $<"
	$(CC) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -I.

# Правило для всех ASM файлов
src/%.o: src/%.asm
	@echo "-> Компиляция ASM: $<"
	$(AS) $< -f elf -o $@

# --- Цели Запуска и Очистки ---

iso: $(KERNEL_BIN) $(GRUB_CFG)
	@echo "-> ISO: Создание загрузочного образа"

	# Убеждаемся, что папка существует
	mkdir -p iso_root/boot/grub

	# 1. Копируем ЯДРО в корень boot (используем ELF для multiboot!)
	cp $(KERNEL_ELF) iso_root/boot/kernel.bin

	# 2. Копируем grub.cfg в папку grub
	cp $(GRUB_CFG) iso_root/boot/grub/grub.cfg

	# Создаем ISO образ
	grub-mkrescue -o myos.iso iso_root

run: iso
	@echo "-> QEMU: Запуск ядра..."
	$(QEMU) -cdrom myos.iso

# ... (Остальные цели остались без изменений)

clean:
	@echo "-> Очистка..."
	rm -f $(OBJECTS) $(KERNEL_ELF) $(KERNEL_BIN) myos.iso