.PHONY: all

INCLUDE_DIR := lib/goblibc/include
ESTROS_INCLUDE_DIR := lib/estros/include

CC := x86_64-elf-gcc
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -I $(INCLUDE_DIR) -I $(ESTROS_INCLUDE_DIR)

BUILD_DIR := build/lib/goblibc
SOURCE_DIR := lib/goblibc/src

LIB_NAME := libgoblibc.a

LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

TARGETS := $(BUILD_DIR)/string.c.o\
			$(BUILD_DIR)/stdlib.c.o\
			$(BUILD_DIR)/ctype.c.o\
			$(BUILD_DIR)/atof.c.o\
			$(BUILD_DIR)/floatscan.c.o\
			$(BUILD_DIR)/fmath.c.o\
			$(BUILD_DIR)/format.c.o\
			$(BUILD_DIR)/file.c.o\
			$(BUILD_DIR)/wchar.c.o\
			$(BUILD_DIR)/strerror.c.o\
			$(BUILD_DIR)/scanf.c.o\
			$(BUILD_DIR)/intscan.c.o\
			$(BUILD_DIR)/arith64.c.o\
			$(BUILD_DIR)/mkinternal.c.o\
			$(BUILD_DIR)/estros.c.o\
			$(BUILD_DIR)/file_scan_helpers.c.o\
			$(BUILD_DIR)/app.c.o

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




