#include "ata.h"
#include "inboutb.h"

void insw(uint16_t port, void* addr, int count)
{
    asm volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void outsw(uint16_t port, void* addr, int count)
{
    asm volatile("rep outsw" : "+S"(addr), "+c"(count) : "d"(port));
}


int ata_wait(void)
{
    for (int i = 0; i < 1000000; i++) {
        uint8_t status = inb(ATA_STATUS);

        if (!(status & 0x80)) { // BSY = 0
            if (status & 0x01) {
                return 1;
            }
            return 0;
        }
    }

    return 0;
}



void ata_read_sector(uint32_t lba, uint8_t* buffer)
{
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F)); // 0xE0 = LBA mode, master drive
    outb(ATA_SECCOUNT, 1); // read 1 sector
    outb(ATA_LBA_LOW, (uint8_t)(lba));
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_CMD, 0x20); // read sectors with retry

    ata_wait();

    insw(ATA_DATA, buffer, 256); // 256 words = 512 bytes
}

#include <print.h>
void ata_write_sector(uint32_t lba, uint8_t* buffer)
{
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)(lba));
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_CMD, 0x30); // write sectors with retry

    ata_wait();

    outsw(ATA_DATA, buffer, 256); // 512 bytes = 256 words

    // Send cache flush
    outb(ATA_CMD, 0xE7);
    ata_wait();
}
