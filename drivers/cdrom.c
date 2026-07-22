#include "cdrom.h"

#include "../include/types.h"
#include "io.h"

extern void print(char *text);

#define PRIMARY_IO      0x1F0
#define SECONDARY_IO    0x170

#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_SECTOR      2
#define ATA_LBA_LOW     3
#define ATA_LBA_MID     4
#define ATA_LBA_HIGH    5
#define ATA_DEVICE      6
#define ATA_STATUS      7
#define ATA_COMMAND     7

#define ATA_IDENTIFY        0xEC
#define ATA_PACKET          0xA0

static void ide_delay(void)
{
    io_wait();
    io_wait();
    io_wait();
    io_wait();
}

static uint8_t ide_identify(uint16_t io, uint8_t drive)
{
    uint8_t status;
    uint8_t mid;
    uint8_t high;

    outb(io + ATA_DEVICE, 0xA0 | (drive << 4));

    ide_delay();

    outb(io + ATA_SECTOR, 0);
    outb(io + ATA_LBA_LOW, 0);
    outb(io + ATA_LBA_MID, 0);
    outb(io + ATA_LBA_HIGH, 0);

    outb(io + ATA_COMMAND, ATA_IDENTIFY);

    status = inb(io + ATA_STATUS);

    if(status == 0)
    {
        return 0;
    }

    while(status & 0x80)
    {
        status = inb(io + ATA_STATUS);
    }

    mid = inb(io + ATA_LBA_MID);
    high = inb(io + ATA_LBA_HIGH);

    if(mid == 0x14 && high == 0xEB)
    {
        return 2; /* ATAPI */
    }

    if(status & 0x01)
    {
        return 0;
    }

    return 1; /* ATA */
}

static void print_device(uint16_t io, uint8_t drive, char *name)
{
    uint8_t type = ide_identify(io, drive);

    print(name);

    if(type == 0)
    {
        print(": Empty\n");
    }
    else if(type == 1)
    {
        print(": ATA Hard Disk\n");
    }
    else
    {
        print(": ATAPI CD-ROM\n");
    }
}

static bool atapi_packet_test(void)
{
    uint8_t status;

    /* Secondary Master = CD-ROM */
    outb(SECONDARY_IO + ATA_DEVICE, 0xA0);

    ide_delay();

    outb(SECONDARY_IO + ATA_COMMAND, ATA_PACKET);

    status = inb(SECONDARY_IO + ATA_STATUS);

    if(status == 0)
    {
        return false;
    }

    while(status & 0x80)
    {
        status = inb(SECONDARY_IO + ATA_STATUS);
    }

    return true;
}

void cdrom_init(void)
{
    print("IDE DEVICE IDENTIFY\n");

    print_device(PRIMARY_IO, 0, "Primary Master");
    print_device(PRIMARY_IO, 1, "Primary Slave");
    print_device(SECONDARY_IO, 0, "Secondary Master");
    print_device(SECONDARY_IO, 1, "Secondary Slave");

    print("IDENTIFY COMPLETE\n");

    if(atapi_packet_test())
    {
        print("ATAPI PACKET OK\n");
    }
    else
    {
        print("ATAPI PACKET FAILED\n");
    }
}

bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    (void)sector;
    (void)buffer;

    return false;
}
