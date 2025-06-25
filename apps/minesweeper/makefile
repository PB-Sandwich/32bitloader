PROJECT_NAME := minesweeper
MAIN_FILE := minesweeper.asm
APP_START := 0x300000

.PHONY: all
all:
	mkdir -p build/apps/$(PROJECT_NAME)
	nasm -f elf apps/$(PROJECT_NAME)/$(MAIN_FILE) -o build/apps/$(PROJECT_NAME).o

	ld -m elf_i386 -nostdlib -Ttext=$(APP_START) -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME).o
	
	cp build/apps/$(PROJECT_NAME).elf build/root/
