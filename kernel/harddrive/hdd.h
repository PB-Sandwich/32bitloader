
#pragma once

#include <stdint.h>

#define SECTOR_SIZE 512

void hdd_read(uint32_t sector, uint32_t n_sectors, void* buffer);
void hdd_write(uint32_t sector, uint32_t n_sectors, void* buffer);

