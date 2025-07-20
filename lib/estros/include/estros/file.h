#ifndef ESTROS_FILE_H
#define ESTROS_FILE_H

#include <stdint.h>

typedef enum
{
    ESTROS_BEG = 1,
    ESTROS_CUR = 2,
    ESTROS_END = 3,
} Whence;

typedef enum
{
    ESTROS_READ = 0b1,
    ESTROS_WRITE = 0b01,
    ESTROS_APPEND = 0b001,
} FileFlags;

typedef struct
{
    uint32_t type;
    uint32_t size;
    void *(*open)(void *inode, FileFlags flags); // returns VFSFile
    void (*close)(void *file);
    uint32_t (*read)(void *file, void *buffer, uint32_t buffer_size); // returns the number of bytes read/written
    uint32_t (*write)(void *file, void *buffer, uint32_t buffer_size);
    void (*ioctl)(void *file, uint32_t *command, uint32_t *arg);
    void (*seek)(void *file, uint32_t offset, Whence whence);
    uint32_t (*tell)(void *file);
    void (*flush)(void *file);
    void *private_data;
    uint32_t number_of_references;
} IndexNode;

typedef struct
{
    IndexNode *inode;
    void *private_data;
    uint32_t private_data_size;
    uint32_t position;
} File;

static inline File *open_file(char *path, FileFlags flags)
{
    File *ret = 0;
    __asm__ volatile("int $0x40\n\t" : "=b"(ret) : "a"(2), "b"(path), "c"(flags));
    return ret;
}

static inline void close_file(File *file)
{
    __asm__ volatile("int $0x40\n\t" ::"a"(3), "b"(file));
}

static inline uint32_t read_file(File *file, void *buffer, uint32_t buffer_size)
{
    uint32_t ret;
    __asm__ volatile("int $0x40\n\t" : "=a"(ret) : "a"(4), "b"(file), "c"(buffer_size), "d"(buffer));
    return ret;
}

static inline uint32_t write_file(File *file, void *buffer, uint32_t buffer_size)
{
    uint32_t ret;
    __asm__ volatile("int $0x40\n\t" : "=a"(ret) : "a"(5), "b"(file), "c"(buffer_size), "d"(buffer));
    return ret;
}

static inline void ioctl(File *file, uint32_t *command, uint32_t *arg)
{
    __asm__ volatile("int $0x40\n\t" ::"a"(6), "b"(file), "c"(command), "d"(arg));
}

static inline void seek_file(File *file, uint32_t offset, Whence whence)
{
    __asm__ volatile("int $0x40\n\t" ::"a"(7), "b"(file), "c"(offset), "d"(whence));
}

static inline uint32_t tell_file(File *file)
{
    uint32_t ret;
    __asm__ volatile("int $0x40\n\t" : "=c"(ret) : "a"(8), "b"(file));
    return ret;
}

static inline uint32_t create_file(char *path)
{
    uint32_t ret;
    __asm__ volatile("int $0x40\n\t" : "=a"(ret) : "a"(9), "b"(path));
    return ret;
}

#endif
