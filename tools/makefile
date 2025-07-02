
.PHONY: all

BUILD_DIR = build/tools
SOURCE_DIR = tools
TARGETS = $(BUILD_DIR)/EstrOSFS

all: $(TARGETS)

$(BUILD_DIR)/%: $(SOURCE_DIR)/%.cpp
	@mkdir -p ./build/tools
	clang++ -o $@ $< -g
