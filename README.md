# x86 32 Bit Loader

Loads a x86 32 bit app into memory address 0x300008 (3Mb) and provides some basic syscalls.

## Making an App

The makefile has the variable APP_BIN which you set to the path of your apps binary, it should be linked for memory address 0x300008.
It also has a variable APP_ENTRY which lets you define the offset into where it gets loaded (address 0x300008) that the entry function is.
The app will run at ring 0 (most privileged).

## Kernel Exports
The app will get passed the following at esp + 8
```c
void printf(const char* fmt, ...);
void print_char(uint8_t chr);
void print_string(uint8_t* str);
void set_color(uint8_t fg, uint8_t bg);
void newline();
void clear();
uint8_t get_cursor_pos(); // high order are x, low order are y
void set_cursor_pos(uint8_t pos); // high order are x, low order are y
void clear_key_pressed(); // clears the key pressed flag
uint8_t key_pressed(); // returns 1 if a key is pressed
uint8_t scancode(); // gets the last scancode
void set_keyboard_function(void (*keyboard_function_)(uint8_t scancode));
void ata_read_sector(uint32_t lba, uint8_t* buffer);
void ata_write_sector(uint32_t lba, uint8_t* buffer);
struct IDTEntry make_idt_entry(uint32_t *offset, uint16_t selector, uint8_t type_attr);
void memcpy(void *dest, void *source, uint32_t size);
```

## Syscalls

All syscalls are on interrupt 0x40 (decimal 64).
The value in `eax` will specify the operation (see table below).

### 0x00
NOP

### 0x01
Clears the text buffer.

### 0x02
Prints the string pointed to by `ebx`.

### 0x03
Returns info about the text buffer (location, `ebx`; width, `ecx`; height `edx`).

### 0x04
Sets the cursor pos to x `ebx` and y `ecx`. 

### 0x05
Returns the last key pressed in `ebx`.

### 0x06
Lets you set a function the keyboard handler will call and pass the scancode to (`void keyboard(uint8_t scancode)`).
The pointer to the function is passed in `ebx`.

### 0x07
Reads sectors (one sector = 512 bytes) from the hard drive. 
`ebx` the sector offset to read from.
`ecx` is the number of sectors to read.
`edx` is the memory location to write to.

### 0x08
Writes sectors (one sector = 512 bytes) to the hard drive. 
`ebx` the sector offset to write to.
`ecx` is the number of sectors to write.
`edx` is the memory location to read from.

