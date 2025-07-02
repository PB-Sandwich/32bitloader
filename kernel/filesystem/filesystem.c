
#include <stdint.h>

enum {
    FS_REGULAR_FILE = 1,
    FS_BLOCK_DEVICE = 2,
    FS_CHARACTER_DEVICE = 3,
};

typedef struct {
    uint8_t type;
    uint32_t size;
    uint32_t blocks[15];
} Virtual_File;
