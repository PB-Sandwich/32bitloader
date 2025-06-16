
# the app that it will load and its entry point offset
APP_BIN = build/examples/main.bin
APP_ENTRY = 0

APP_SIZE=$$(stat --format="%s" $(APP_BIN))

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
		$(BUILD_DIR)/kernel/ata.c.o \
		$(BUILD_DIR)/kernel/interrupts/system_calls.c.o

TARGETS_BIN := $(BUILD_DIR)/kernel/boot.bin
TARGETS := $(TARGETS_ELF) $(TARGETS_BIN)

CC := i386-elf-gcc
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g
LINKER := i386-elf-ld

QEMU_FLAGS := -m 512M

.PHONY: all clean run

example:
	make --file examples/makefile all

run-ex: example run

run: all
	qemu-system-x86_64 -hda $(BUILD_DIR)/$(NAME).img $(QEMU_FLAGS)

all: $(TARGETS)
	$(LINKER) -nostdlib -T linker.ld -o $(BUILD_DIR)/kernel.elf $(TARGETS_ELF)
	objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
	cat $(BUILD_DIR)/kernel/boot.bin $(BUILD_DIR)/kernel.bin > $(BUILD_DIR)/$(NAME).img

	# fill to 0xffff
	truncate -s 65536 $(BUILD_DIR)/$(NAME).img
	printf "%08x" $(APP_SIZE) | sed 's/\(..\)\(..\)\(..\)\(..\)/\4\3\2\1/' | xxd -r -p >> $(BUILD_DIR)/$(NAME).img
	printf "%08x" $(APP_ENTRY) | sed 's/\(..\)\(..\)\(..\)\(..\)/\4\3\2\1/' | xxd -r -p >> $(BUILD_DIR)/$(NAME).img

	cat $(APP_BIN) >> $(BUILD_DIR)/$(NAME).img

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


