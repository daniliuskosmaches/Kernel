# --- Настройка путей и имен файлов ---
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
QEMU = qemu-system-i386

# Список исходных файлов
AS_SOURCES = entry.asm
C_SOURCES  = kernel.c

# Целевые объектные файлы (ВАЖНО: они будут в папке src/)
OBJECTS = $(patsubst %.asm, src/%.o, $(AS_SOURCES)) $(patsubst %.c, src/%.o, $(C_SOURCES))

LINKER_SCRIPT = linker.ld
KERNEL_BIN = kernel.bin
GRUB_CFG = iso_root/boot/grub/grub.cfg

# --- Правила сборки ---

# Правило для C: из src/%.c в src/%.o
src/%.o: src/%.c
	@echo "-> Компиляция C: $<"
	$(CC) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -I.

# Правило для Ассемблера: из src/%.asm в src/%.o
src/%.o: src/%.asm
	@echo "-> Компиляция ASM: $<"
	$(AS) $< -f elf -o $@