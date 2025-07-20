PROJECT_NAME := imagedisplay
.PHONY: all

LIBC_PATH := ./build/lib/goblibc
LIBC_NAME := goblibc
LIBC_INCLUDE_DIR := lib/goblibc/include
ESTROS_INCLUDE_DIR := -I ./lib/estros/include/
ESTROS_PATH := -I ./lib/estros/include/

CC := gcc
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -fno-stack-protector -I $(LIBC_INCLUDE_DIR) -I ./lib/estros/include/
LD := ld
LDFLAGS := -m elf_i386 -nostdlib -T apps/$(PROJECT_NAME)/linker.ld

all:
	mkdir -p build/apps/$(PROJECT_NAME)
	$(CC) $(CFLAGS) -c -o build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o apps/$(PROJECT_NAME)/main.c

	$(LD) $(LDFLAGS) -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o -L$(LIBC_PATH) -l$(LIBC_NAME)
	
