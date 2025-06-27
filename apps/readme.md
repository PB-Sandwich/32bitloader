# Apps
These are user facing apps that will be built and loaded into the operating system image

# Building an app
    Each app should be placed in it's own folder with a custom makefile, for it to then be included in final image it should copy the built binary into `build/root/`
    Example:
    `cp build/apps/$(PROJECT_NAME).elf build/root/`
    Apps need to be built without using standard library(see `Using libc` for alternative) and target `elf_i386`. An example of a makefile for different languages is provided below.
    To exit the app, simply return from the entry function.
## Using libc
    A custom implementation of libc(goblibc) is built before apps and produces a `libgoblibc.a` file in `build/lib` directory
    To link to the goblibc add `-L./build/lib/goblibc -lgoblibc` to the linker invocation
## Assembly

    ```makefile
    PROJECT_NAME := your_app_name
    MAIN_FILE := your_app_name.asm
    # apps must start at this location
    APP_START := 0x300000

    .PHONY: all
    all:
        # create folder for the build files
        mkdir -p build/apps/$(PROJECT_NAME)
        # assemble the file
        nasm -f elf apps/$(PROJECT_NAME)/$(MAIN_FILE) -o build/apps/$(PROJECT_NAME).o

        ld -m elf_i386 -nostdlib -Ttext=$(APP_START) -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME).o
        # copy the app into the filesystem root
        cp build/apps/$(PROJECT_NAME).elf build/root/

    ```
## C
    An example of a simple c program makefile that links to goblibc

    ```makefile

    PROJECT_NAME := your_app_name
    .PHONY: all

    LIBC_PATH := ./build/lib/goblibc
    LIBC_NAME := goblibc
    LIBC_INCLUDE_DIR := lib/goblibc/include

    CC := clang
    CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -fno-stack-protector -I $(LIBC_INCLUDE_DIR)
    LD := ld
    LDFLAGS := -m elf_i386 -nostdlib -T apps/$(PROJECT_NAME)/linker.ld

    all:
        mkdir -p build/apps/$(PROJECT_NAME)
        $(CC) $(CFLAGS) -c -o build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o apps/$(PROJECT_NAME)/main.c
        # Link the built program code files into an elf file 
        $(LD) $(LDFLAGS) -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o -L$(LIBC_PATH) -l$(LIBC_NAME)
        # copy the app into the filesystem root
        cp build/apps/$(PROJECT_NAME).elf build/root/
    ```

    An example linker file that will be required for the build process
    ```ld
    ENTRY(app_main)

    SECTIONS {
        . = 0x300000;

        .text.start ALIGN(4K) : {
            *(.text.start)
            *(.text*)
        }

        .rodata ALIGN(4K) : { *(.rodata*) }
        .data ALIGN(4K) : { *(.data*) }
        .bss ALIGN(4K) : { *(.bss*) }
    }
    ```

    An entry function for a c program should look like this
    ```c
        int app_main(struct KernelExports *kernel_exports)
    ```
