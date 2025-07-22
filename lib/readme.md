# Libraries

This folder contains libraries built specifically for this operating system. This includes reimplementations of standard libraries like libc and some more specific stuff, like OS libraries and anything else that might come in handy

# Building custom libraries

Currently there is no automated way to include library in the build process, so you will have to modify main makefile. Go to the lib section and add call to your library makefile
```makefile
libs:
    # your makefile here
	@echo "Building standard c library"
	make --file lib/goblibc/makefile all
```

Generally libraries should be built as static, but this could be changed in the future. Structure of the library should include `include` folder and `src`, to be exposed to other programs. For example:
```
lib
    yourlib
        include
        src
        makefile
        readme.md
```
Libraries should be build using `gcc` and archived using `ar`. An example makefile:
```makefile 
.PHONY: all

INCLUDE_DIR := lib/yourlibname/include
CC := x86_64-elf-gcc
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -I $(INCLUDE_DIR)
BUILD_DIR := build/lib/yourlibname
SOURCE_DIR := lib/yourlibname/src

LIB_NAME := libyourlibname.a

LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

TARGETS := $(BUILD_DIR)/filenamehere.c.o

ARCHIVER := x86_64-elf-gcc-ar
ARCHIVER_FLAGS := rcs

all: $(TARGETS)
	@echo "Building c lib"
	@mkdir -p $(BUILD_DIR)
	$(ARCHIVER) $(ARCHIVER_FLAGS) $(LIB_PATH) $(TARGETS)

$(BUILD_DIR)/%.c.o: $(SOURCE_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
```

Place final `.a` file in the `build/lib` folder where it can be picked up by other apps
