.PHONY: all

INCLUDE_DIR := lib/estros/include
CC := clang
CFLAGS := -nostdlib -ffreestanding -Wall -Wextra -g -m32 -fno-stack-protector -msoft-float -I $(INCLUDE_DIR)
BUILD_DIR := build/lib/estros
SOURCE_DIR := lib/estros/src

LIB_NAME := libestros.a

LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

TARGETS := 

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




