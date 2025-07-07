.PHONY: all

INCLUDE_DIR := lib/goblibc/include
CC := clang
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -fno-stack-protector -msoft-float -I $(INCLUDE_DIR)
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
			$(BUILD_DIR)/arith64.c.o\
			$(BUILD_DIR)/app.c.o

ARCHIVER := ar
ARCHIVER_FLAGS := rcs

all: $(TARGETS)
	@echo "Building c lib"
	@mkdir -p $(BUILD_DIR)
	$(ARCHIVER) $(ARCHIVER_FLAGS) $(LIB_PATH) $(TARGETS)

$(BUILD_DIR)/%.c.o: $(SOURCE_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<




