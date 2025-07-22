PROJECT_NAME := test_helper
.PHONY: all

all:
	mkdir -p build/apps/$(PROJECT_NAME)
	clang -nostdlib -ffreestanding -g -m32 -fno-stack-protector -c -o build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o apps/$(PROJECT_NAME)/main.c

	ld -m elf_i386 -nostdlib -T apps/linker.ld -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o
	
