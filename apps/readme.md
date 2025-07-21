# Apps
This folder contains code for "system" apps aka apps that will be included the final operating system image

# Building an app
    Each app has to follow few rules:
    * Folder name mut match the final executable name
    * Each folder should build an "elf" file
    * The app build process should not include moving final executables anywhere
    * There should be at least one makefile that calls whatever build tools are necessary 
    This way the build process will be able to remain fairly modular and easy to update
## Build process rules
    Apps *must* create a freestanding executable with no standard library(see "Using libc" on alternatives) and target `elf_i386` when linking. The entry function must be placed at `0x300000`(see example linker file). To exit the app, simply return from the entry function.
    
    Currently only C and assembly have been tested and C++ without STL could work as long as long a `new` or `delete` are not used. 
    ***IMPORTANT!*** To make sure the app is built during the building process, include it's name in the main makefile(located at `apps/makefile`) in the `APPS` list. It should follow `$(DESTINATION_APP_DIR)/APPNAMEHERE.$(EXE_EXT)` template to ensure compatibility with current build process.
## Libraries
     Currently only a few libraries are provided, but they will include either reimplementations of standard libraries or something more specific to the project.
### Using libc
    A custom implementation of libc(goblibc) is built before apps and produces a `libgoblibc.a` file in `build/lib` directory
    To link to the goblibc add `-L./build/lib/goblibc -lgoblibc` to the linker invocation
### EstrOS library
    This library provides OS specific(think `windows.h`) functionality and can be used to interact with specific parts of the OS.
## Debugging
    To debug an app using `gdb` launch gdb with app elf file as argument, located in the `build/apps` folder.  For example `gdb build/apps/app.elf`. And use `make debug` instead of `make all run` when launching the whole OS to put it in the debug mode. Then in `gdb` use `target remote localhost:1234` to connect to QEMU.
## Examples and templates
### Assembly

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

    ```
## C
    An example of a simple c program makefile that links to goblibc. This assumes that you are only using one file and it's called `main.c`

    ```makefile

   PROJECT_NAME := yourappnamehere
    .PHONY: all

    LIBC_PATH := ./build/lib/goblibc
    LIBC_NAME := goblibc
    LIBC_INCLUDE_DIR := lib/goblibc/include

    CC := x86_64-elf-gcc
    CFLAGS := -m32 -nostdlib -ffreestanding -Wall -Wextra -g -fmerge-constants -I $(LIBC_INCLUDE_DIR)
    LD := x86_64-elf-ld
    LDFLAGS := -m elf_i386 -nostdlib -T apps/$(PROJECT_NAME)/linker.ld

    all:
        mkdir -p build/apps/$(PROJECT_NAME)
        $(CC) $(CFLAGS) -c -o build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o apps/$(PROJECT_NAME)/main.c

        $(LD) $(LDFLAGS) -o build/apps/$(PROJECT_NAME).elf build/apps/$(PROJECT_NAME)/$(PROJECT_NAME).o -L$(LIBC_PATH) -l$(LIBC_NAME)
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
        int main(){/*your code here*/}
    ```
