# --- Настройка Инструментов ---
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
QEMU = qemu-system-i386

# --- Файлы и Пути ---
# Добавляем -Iinclude, чтобы можно было писать #include "core/task.h"
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -Iinclude -Iinclude/arch -Iinclude/core -Iinclude/drivers -Iinclude/lib

# Автоматический поиск всех .c и .asm файлов в подпапках src/
C_SOURCES = $(shell find src -name "*.c")
ASM_SOURCES = $(shell find src -name "*.asm")

# Превращаем пути src/xxx.c в obj/xxx.o
OBJECTS = $(patsubst src/%.c, obj/%.o, $(C_SOURCES)) \
          $(patsubst src/%.asm, obj/%.o, $(ASM_SOURCES))

LINKER_SCRIPT = linker.ld
KERNEL_ELF    = kernel.elf
KERNEL_BIN    = kernel.bin
ISO_NAME      = myos.iso

# --- Главные Цели ---
all: clean $(KERNEL_BIN) iso

# Правило для создания папок в obj/ (чтобы структура повторилась)
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "-> Compiling C: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: src/%.asm
	@mkdir -p $(dir $@)
	@echo "-> Assembling ASM: $<"
	@$(AS) -f elf $< -o $@

# Линковка
$(KERNEL_BIN): $(OBJECTS)
	@echo "-> Linking Kernel..."
	@$(LD) -T $(LINKER_SCRIPT) -o $(KERNEL_ELF) $(OBJECTS)
	@i686-elf-objcopy -O binary $(KERNEL_ELF) $(KERNEL_BIN)

iso: $(KERNEL_BIN)
	@echo "-> Creating ISO..."
	@mkdir -p iso_root/boot/grub
	@cp $(KERNEL_ELF) iso_root/boot/kernel.bin
	@cp grub.cfg iso_root/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO_NAME) iso_root

run: iso
	@$(QEMU) -cdrom $(ISO_NAME) -serial stdio # Вывод логов в терминал

clean:
	@echo "-> Cleaning up..."
	@rm -rf obj $(KERNEL_ELF) $(KERNEL_BIN) $(ISO_NAME) iso_root