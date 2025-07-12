# x86 32 Bit EstrOS 

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
```
NOP
```

### 0x01 get app info
```
output
eax = safe heap space
ebx = stdout
ecx = stdin
edx = stderr
```

### 0x02
```
NOP
```

### 0x03 read
```
input
ebx = file pointer
ecx = size
edx = buffer
output
ecx = number of bytes read
```

### 0x04 write
```
input
ebx = file pointer
ecx = size
edx = buffer
output
ecx = number of bytes written
```

## File System
EstrOS File System v1
(This definitely is not mostly copied from ext2)

Block Size = 1024 bytes

File Types:
File = 1
Directory = 2

Bit Maps:
0 = free
1 = used

General Structure
offset       | length                                   | description
-------------|------------------------------------------|-------------
 0x10000     | 0x0c                                     | Super Block
 +block_size | 0xffffffff / block_size / 8              | free block bit map
 +last length| max_inodes * inode_size / block_size / 8 | free inode bit map
 +last length| inodes_max * inode_size / block_size     | Index Nodes
 +last length| ....                                     | Data Blocks

Super Block @ 0x10000 on the drive
offset | length | description
-------|--------|-------------
0x00   | 0x08   | Number of bytes in the filesystem
0x08   | 0x04   | Number of blocks in the filesystem
0x0c   | 0x04   | Max number of index nodes in the filesystem
0x10   | 0x04   | Number of inodes in the filesystem
0x14   | 0x04   | root index node number

Index Node (64 bytes)
offset | length | description
-------|--------|-------------
0x00   | 0x04   | type
0x04   | 0x04   | size
0x08   | 0x04   | block0
0x0c   | 0x04   | block1
0x10   | 0x04   | block2
0x14   | 0x04   | block3
0x18   | 0x04   | block4
0x1c   | 0x04   | block5
0x20   | 0x04   | block6
0x24   | 0x04   | block7
0x28   | 0x04   | block8
0x2c   | 0x04   | block9
0x30   | 0x04   | block10 (single indirect)
0x34   | 0x04   | block11 (double indirect)
0x38   | 0x04   | reserved 
0x3c   | 0x04   | reserved

Directory Entry
offset | length | description
-------|--------|-------------
0x00   | 0x04   | index node number
0x04   | 0x04   | entry length
0x08   | 0x04   | name length
0x0c   | ....   | name[] (not null terminated)

