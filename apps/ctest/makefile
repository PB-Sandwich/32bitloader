PROJECT_NAME := ctest
.PHONY: all

LIBC_PATH := ./build/lib/goblibc
LIBC_NAME := goblibc
LIBC_INCLUDE_DIR := lib/goblibc/include
ESTROS_INCLUDE_DIR := ./lib/estros/include/
ESTROS_PATH := ./build/lib/estros
ESTROS_NAME := estros
LINKER_SCRIPT_PATH := apps/linker.ld


CC := x86_64-elf-gcc
CFLAGS := -m32 -nostdlib -ffreestanding -Wall -Wextra -g -fmerge-constants -I $(LIBC_INCLUDE_DIR) -I $(ESTROS_INCLUDE_DIR)
LD := x86_64-elf-ld
LDFLAGS := -m elf_i386 -nostdlib -T $(LINKER_SCRIPT_PATH)

all:
	mkdir -p build/apps/$(PROJECT_NAME)
	$(CC) $(CFLAGS) -c -o build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o apps/$(PROJECT_NAME)/main.c

	$(LD) $(LDFLAGS) -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o -L$(LIBC_PATH) -l$(LIBC_NAME) -L$(ESTROS_PATH) -l$(ESTROS_NAME)
	
