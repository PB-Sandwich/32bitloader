
.PHONY: all
all:
	@mkdir -p build/apps/$(PROJECT_NAME)
	@mkdir -p ./build/root/

	make --file apps/old_demo/makefile all
	make --file apps/test_helper/makefile all
	make --file apps/minesweeper/makefile all
	make --file apps/brainfuck/makefile all
	
