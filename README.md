# x86 32 Bit Loader

Loads a x86 32 bit app into memory address 0x300008 (3Mb) and provides some basic syscalls.

## Making an App

The makefile has the variable APP_BIN which you set to the path of your apps binary path, it should be linked for memory address 0x300008.
It also has a variable APP_ENTRY which lets you define the offset into where it gets loaded (address 0x300008) that the entry function is.
The app will run at ring 0 (most privileged).

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
NOP

### 0x05
Waits for a key press and returns it in `ebx`.

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
