
#include "memutils.h"
#include <stdint.h>

void memcpy(void* dest, void* source, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)source)[i];
    }
}
