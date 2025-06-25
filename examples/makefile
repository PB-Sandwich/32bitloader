
.PHONY: all

all:
	mkdir -p build/examples/

	nasm -f elf examples/main.asm -o build/examples/old-app.o
	nasm -f elf examples/minesweeper.asm -o build/examples/minesweeper.o

	clang -nostdlib -ffreestanding -g -m32 -fno-stack-protector -c -o build/examples/main.o examples/main.c

	ld -m elf_i386 -nostdlib -T examples/linker.ld -o build/examples/app.elf build/examples/main.o
	ld -m elf_i386 -nostdlib -Ttext=0x300000 -o build/examples/app-old.elf build/examples/old-app.o
	ld -m elf_i386 -nostdlib -Ttext=0x300000 -o build/examples/minesweeper.elf build/examples/minesweeper.o
	
	@mkdir -p ./build/root/
	cp build/examples/app.elf build/root/
	cp build/examples/app-old.elf build/root/
	cp build/examples/minesweeper.elf build/root/
