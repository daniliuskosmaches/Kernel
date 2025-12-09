# --- Настройка Инструментов ---
# Используем 32-битный кросс-компилятор, как и планировали
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
QEMU = qemu-system-i386

# --- Файлы и Пути ---
# Явно указываем, что объектные файлы будут в подпапке src/
OBJECTS      = src/entry.o src/kernel.o
LINKER_SCRIPT = linker.ld
KERNEL_BIN   = kernel.bin
GRUB_CFG     = src/iso_root/grub/grub.cfg

# --- Главные Цели ---

.PHONY: all iso run clean

all: $(KERNEL_BIN) iso

# Компоновка: Зависит от всех объектных файлов и скрипта линковщика
$(KERNEL_BIN): $(OBJECTS) $(LINKER_SCRIPT)
	@echo "-> Линковка: Создание $@"
	$(LD) -T $(LINKER_SCRIPT) -o $@ $(OBJECTS)

# --- Правила Компиляции (с явным указанием пути src/) ---

# Правило для C: src/kernel.c -> src/kernel.o
src/%.o: src/%.c
	@echo "-> Компиляция C: $<"
	$(CC) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -I.

# Правило для Ассемблера: src/entry.asm -> src/entry.o
src/%.o: src/%.asm
	@echo "-> Компиляция ASM: $<"
	$(AS) $< -f elf -o $@

# --- Цели Запуска и Очистки ---

iso: $(KERNEL_BIN) $(GRUB_CFG)
	@echo "-> ISO: Создание загрузочного образа"
	mkdir -p iso_root/boot/
	cp $(KERNEL_BIN) iso_root/boot/
	grub-mkrescue -o myos.iso iso_root

run: iso
	@echo "-> QEMU: Запуск ядра..."
	$(QEMU) -cdrom myos.iso

clean:
	@echo "-> Очистка..."
	rm -f $(OBJECTS) $(KERNEL_BIN) myos.iso
	rm -rf iso_root/boot