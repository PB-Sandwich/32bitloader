# GobLibC

Custom reimplementation of libc by Sofia "MetalPizzaCat" based on https://github.com/kraj/musl, openbsd and android source code. This is incomplete and messy. Please fix.

# Notes

While i try to readd functions as faithfully as i can, it often isn't possible either due to specifics or either because c is just a tedious language or just because i have no clue how. This means stuff either will be copied and adapted from other implementations or just going to be sort of remade based on presumed logic. The actual libc is big and often consists of many levels of function calls all of which would need to be implemented. As such file organization can be off, please fix if you find misplaced function. **MOST IMPORTANTLY** there are some functions which *have* a version that can fulfill their goal, but i wasn't sure were good enough to replace the actual function, so i gave them `gob_` prefix. These might change and either be replaced with ones that fit libc specs. *OR* the specs one will be added and `gob_` ones will be used for something else 