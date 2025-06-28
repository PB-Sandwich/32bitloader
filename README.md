# x86 32 Bit Loader

Loads a x86 32 bit app into memory address 0x300000 (3Mb) and provides some basic syscalls.

## Making an App

The app should be put into the ./build/root directory as a statically linked elf file for 0x300000.

## Kernel Exports
The app will get passed the following at esp + 8
```c
void printf(const char* fmt, ...);

void print_char(uint8_t chr);
void print_string(uint8_t* str);
void set_color(uint8_t fg, uint8_t bg);
void newline();
void clear();
void get_cursor_pos(uint8_t* x, uint8_t* y);
void set_cursor_pos(uint8_t x, uint8_t y);

void clear_key_pressed(); // clears the key pressed flag
uint8_t key_pressed(); // returns 1 if a key is pressed
uint8_t scancode(); // gets the last scancode
void set_keyboard_function(void (*keyboard_function_)(uint8_t scancode));

enum Keycode wait_for_keypress();
uint8_t* get_line(); // returns the pointer to input buffer (which will get overwritten on next call)
uint8_t keycode_to_ascii(enum Keycode kc);
enum Keycode scancode_to_keycode(uint8_t sc);

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

## File System
B32FS
Basic 32 bit File system

All strings in the file system are ascii encoded and null-terminated.

File System Data Struct:
offset | length | name
-------|--------|-----
0x00   | 0x04   | root descriptor
0x04   | 0x04   | fs_size 
fs_size is the current size the file system takes on disk
should be in the first sector of the filesystem

File System Descriptor Type:
Value | type
------|-----
0x00  | NONE
0x01  | Directory
0x02  | File
0x03  | Block Device

Directory Entry:
length | name
-------|-----
0x04   | sector of next descriptor

if entry = 0, it is not present

512 byte sector: File Descriptor:
offset | length | name
-------|--------|-----
0x00   | 0x01   | type
0x01   | 0x04   | string sector
0x05   | 0x04   | name offset
0x09   | 0x04   | path offset
0x0d   | 0x04   | last accessed by path offset
0x11   | 0x04   | last modified by path offset
0x15   | 0x04   | attributes
0x19   | 0x04   | file size (in bytes)
0x1d   | 0x04   | file fragment size (in sectors)
0x1d   | 0x04   | file sector
...    | ...    | 0

attributes
offset (bits) | length | name
--------------|--------|-----
0             | 1      | executable
1             | 1      | protect location
2             | 30     | 0

if marked with protect location the files location on disk cannot be changed

string sector:
offset | length | name
-------|--------|-----
0x1FB  | 0x04   | link

this will link to further string sector if needed

File:
offset | length | name
-------|--------|-----
...    | ...    | data
EOF    | 0x04   | link to further data (sector)

link Descriptor
offset | length | name
-------|--------|-----
0x00   | 0x04   | size (in sectors)
...    | ...    | data
EOF    | 0x04   | link to further data (sector)

Directory Entry:
offset | length | name
-------|--------|-----
0x00   | 0x04   | file discriptor (sector)
