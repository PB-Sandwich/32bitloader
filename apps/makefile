
.PHONY: all prepare
DESTINATION_APP_DIR := build/root/apps
BUILD_DIR := build/apps
SOURCE_DIR := apps
EXE_EXT := bin

.PRECIOUS: $(BUILD_DIR)/%.elf

# To add new app to be built, make sure that the app makefile builds a .elf file that is placed in BUILD_DIR
APPS := $(DESTINATION_APP_DIR)/old_demo.$(EXE_EXT)\
		$(DESTINATION_APP_DIR)/test_helper.$(EXE_EXT)\
		$(DESTINATION_APP_DIR)/minesweeper.$(EXE_EXT)\
		$(DESTINATION_APP_DIR)/brainfuck.$(EXE_EXT)\
	  	$(DESTINATION_APP_DIR)/new_test.$(EXE_EXT)\
		$(DESTINATION_APP_DIR)/calc.$(EXE_EXT)\
		$(DESTINATION_APP_DIR)/ctest.$(EXE_EXT)

	
all: prepare $(APPS)
	@echo "Build apps"

prepare:
	@rm -rf $(DESTINATION_APP_DIR)
	@rm -rf $(BUILD_DIR)
	
$(BUILD_DIR)/%.elf: $(SOURCE_DIR)/%/makefile
	@echo "Building $<"
	make --file $< all

$(DESTINATION_APP_DIR)/%.$(EXE_EXT): $(BUILD_DIR)/%.elf
	@mkdir -p $(dir $@)
	cp $< $@




	
