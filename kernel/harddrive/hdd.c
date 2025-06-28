#include "hdd.h"
#include "ata.h"
#include <stdint.h>

void hdd_read(uint32_t sector, uint32_t n_sectors, void* buffer)
{
    for (uint32_t i = 0; i < n_sectors; i++) {
        ata_read_sector(sector + i, buffer + i * SECTOR_SIZE);
    }
}

void hdd_write(uint32_t sector, uint32_t n_sectors, void* buffer)
{
    for (uint32_t i = 0; i < n_sectors; i++) {
        ata_read_sector(sector + i, buffer + i * SECTOR_SIZE);
    }
}
