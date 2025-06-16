
NAME := 32bitloader

SOURCE_DIR := .
KERNEL_SOURCE_DIR := $(SOURCE_DIR)/kernel
BUILD_DIR := build

TARGETS_ELF :=  $(BUILD_DIR)/kernel/main.c.o \
		$(BUILD_DIR)/kernel/print.c.o \
		$(BUILD_DIR)/kernel/tty.c.o \
		$(BUILD_DIR)/kernel/idt.c.o \
		$(BUILD_DIR)/kernel/interrupts/error_handlers.int.c.o \
		$(BUILD_DIR)/kernel/inboutb.c.o \
		$(BUILD_DIR)/kernel/memutils.c.o \
		$(BUILD_DIR)/kernel/interrupts/irq_handlers.int.c.o \
		$(BUILD_DIR)/kernel/ata.c.o

TARGETS_BIN := $(BUILD_DIR)/kernel/boot.bin
TARGETS := $(TARGETS_ELF) $(TARGETS_BIN)

CC := i386-elf-gcc
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g
LINKER := i386-elf-ld

QEMU_FLAGS := -m 512M

.PHONY: all clean run

run: all
	qemu-system-x86_64 -hda $(BUILD_DIR)/$(NAME).img $(QEMU_FLAGS)

all: $(TARGETS)
	$(LINKER) -nostdlib -T linker.ld -o $(BUILD_DIR)/kernel.elf $(TARGETS_ELF)
	objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
	cat $(BUILD_DIR)/kernel/boot.bin $(BUILD_DIR)/kernel.bin > $(BUILD_DIR)/$(NAME).img

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/kernel/boot.bin: $(KERNEL_SOURCE_DIR)/boot.asm
	@mkdir -p $(dir $@)
	nasm -f bin -o $@ $<

$(BUILD_DIR)/kernel/%.c.o: $(KERNEL_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# interrupts
$(BUILD_DIR)/kernel/%.int.c.o: $(KERNEL_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -mno-80387 -mno-sse -mno-mmx -c -o $@ $<


$(BUILD_DIR)/kernel/%.asm.o: $(KERNEL_SOURCE_DIR)/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf32 -g -o $@ $<


