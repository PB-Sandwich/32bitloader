
# the path it will make the filesystem from
# the script will use any elf files entry point and make it to a binary file
FILE_SYSTEM = $(BUILD_DIR)/root

APP_SIZE=$$(stat --format="%s" $(APP_BIN))

QEMU := qemu-system-i386
QEMU_FLAGS := -m 512M

NAME := 32bitloader

SOURCE_DIR := .
KERNEL_SOURCE_DIR := $(SOURCE_DIR)/kernel
BUILD_DIR := build

TARGETS_ELF :=  $(BUILD_DIR)/kernel/main.c.o \
		$(BUILD_DIR)/kernel/print.c.o \
		$(BUILD_DIR)/kernel/idt.c.o \
		$(BUILD_DIR)/kernel/inboutb.c.o \
		$(BUILD_DIR)/kernel/memutils.c.o \
		$(BUILD_DIR)/kernel/heap.c.o \
		$(BUILD_DIR)/kernel/terminal/tty.c.o \
		$(BUILD_DIR)/kernel/harddrive/ata.c.o \
		$(BUILD_DIR)/kernel/keyboard/input.c.o \
		$(BUILD_DIR)/kernel/filesystem/filesystem.c.o \
		$(BUILD_DIR)/kernel/interrupts/error_handlers.int.c.o \
		$(BUILD_DIR)/kernel/interrupts/irq_handlers.int.c.o \
		$(BUILD_DIR)/kernel/interrupts/system_calls.c.o \

TARGETS_BIN := $(BUILD_DIR)/kernel/boot.bin
TARGETS := $(TARGETS_ELF) $(TARGETS_BIN)


CC := clang
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -fno-stack-protector -I $(KERNEL_SOURCE_DIR)
LD := ld
LDFLAGS := -m elf_i386 -nostdlib -T linker.ld 


.PHONY: all clean run


run: all
	$(QEMU) -hda $(BUILD_DIR)/$(NAME).img $(QEMU_FLAGS)

debug: all
	$(QEMU) -hda $(BUILD_DIR)/$(NAME).img $(QEMU_FLAGS) -s -S


all: $(TARGETS) 
	@echo "Linking"
	@$(LD) $(LDFLAGS) -o $(BUILD_DIR)/kernel.elf $(TARGETS_ELF)

	@echo "Making raw binary"
	@objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
	@cat $(BUILD_DIR)/kernel/boot.bin $(BUILD_DIR)/kernel.bin > $(BUILD_DIR)/$(NAME).img

	@echo "Building standard c library"
	make --file lib/goblibc/makefile all

	@echo "Building default apps"
	make --file apps/makefile all

	@echo "Preparing disk image"
	@# fill to 0xffff
	@truncate -s 65536 $(BUILD_DIR)/$(NAME).img
	@mkdir -p $(FILE_SYSTEM)
	@echo "-------------------------------------"
	@./makefilesystem.py $(FILE_SYSTEM) $(BUILD_DIR)/fs.img --offset $$((0x10000 / 0x200))
	@echo "-------------------------------------"
	@cat $(BUILD_DIR)/fs.img >> $(BUILD_DIR)/$(NAME).img

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/kernel/boot.bin: $(KERNEL_SOURCE_DIR)/boot.asm
	@echo "Assembling boot loader: $<"
	@mkdir -p $(dir $@)
	@nasm -f bin -o $@ $<

$(BUILD_DIR)/kernel/%.c.o: $(KERNEL_SOURCE_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# interrupts
$(BUILD_DIR)/kernel/%.int.c.o: $(KERNEL_SOURCE_DIR)/%.c
	@echo "Compiling $< as interrupt"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -mno-80387 -mno-sse -mno-mmx -c -o $@ $<


$(BUILD_DIR)/kernel/%.asm.o: $(KERNEL_SOURCE_DIR)/%.asm
	@echo "Assembling $<"
	@mkdir -p $(dir $@)
	@nasm -f elf32 -g -o $@ $<


