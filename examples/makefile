
.PHONY: all

all:
	mkdir -p build/examples/
	nasm -fbin examples/main.asm -o build/examples/old-app.bin
	clang -nostdlib -ffreestanding -g -m32 -fno-stack-protector -c -o build/examples/main.o examples/main.c
	ld -m elf_i386 -nostdlib -T examples/linker.ld -o build/examples/app.elf build/examples/main.o
	@mkdir -p ./build/root/
	cp build/examples/old-app.bin build/root/
	cp build/examples/app.elf build/root/
