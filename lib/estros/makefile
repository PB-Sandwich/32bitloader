.PHONY: all

INCLUDE_DIR := lib/estros/include
CC := x86_64-elf-gcc 
CFLAGS := -m32 -nostdlib -ffreestanding -Wall -Wextra -g -fmerge-constants -I  $(INCLUDE_DIR)
BUILD_DIR := build/lib/estros
SOURCE_DIR := lib/estros/src

LIBC_INCLUDE := ./lib/goblibc/include

LIB_NAME := libestros.a

LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

TARGETS :=	$(BUILD_DIR)/pager.c.o \
		$(BUILD_DIR)/process.c.o

ARCHIVER := x86_64-elf-gcc-ar
ARCHIVER_FLAGS := rcs

all: $(TARGETS)
	@echo "Building estros lib"
	@mkdir -p $(BUILD_DIR)
	$(ARCHIVER) $(ARCHIVER_FLAGS) $(LIB_PATH) $(TARGETS)

$(BUILD_DIR)/%.c.o: $(SOURCE_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $< -I$(LIBC_INCLUDE)

