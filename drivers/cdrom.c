#include "cdrom.h"
#include "io.h"

extern void print(char *text);

#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_FEATURES    0x1F1
#define ATA_SECCOUNT0   0x1F2
#define ATA_LBA0        0x1F3
#define ATA_LBA1        0x1F4
#define ATA_LBA2        0x1F5
#define ATA_HDDEVSEL    0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

#define ATA_CMD_PACKET  0xA0
#define ATAPI_READ10    0x28

#define STATUS_BSY      0x80
#define STATUS_DRQ      0x08
#define STATUS_ERR      0x01

static uint8_t cd_drive = 0xB0;

static bool wait_drq()
{
    int timeout = 1000000;

    while(timeout--)
    {
        uint8_t status = inb(ATA_STATUS);

        if(status & STATUS_ERR)
        {
            print("ATAPI ERR\n");
            return false;
        }

        if(!(status & STATUS_BSY) &&
           (status & STATUS_DRQ))
        {
            return true;
        }
    }

    print("ATAPI TIMEOUT\n");
    return false;
}

void cdrom_init(void)
{
    print("Selecting CD-ROM...\n");

    outb(
        ATA_HDDEVSEL,
        cd_drive
    );

    io_wait();

    outb(
        ATA_FEATURES,
        0
    );

    outb(
        ATA_SECCOUNT0,
        0
    );

    outb(
        ATA_LBA0,
        0
    );

    outb(
        ATA_LBA1,
        0x08
    );

    outb(
        ATA_LBA2,
        0
    );

    print("CD-ROM initialized\n");
}

bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    outb(
        ATA_HDDEVSEL,
        cd_drive
    );

    io_wait();

    outb(
        ATA_FEATURES,
        0
    );

    outb(
        ATA_LBA1,
        0x08
    );

    outb(
        ATA_LBA2,
        0
    );

    outb(
        ATA_COMMAND,
        ATA_CMD_PACKET
    );

    if(!wait_drq())
    {
        print("PACKET failed\n");
        return false;
    }

    uint8_t packet[12];

    for(int i = 0; i < 12; i++)
        packet[i] = 0;

    packet[0] = ATAPI_READ10;

    packet[2] = (sector >> 24) & 0xFF;
    packet[3] = (sector >> 16) & 0xFF;
    packet[4] = (sector >> 8) & 0xFF;
    packet[5] = sector & 0xFF;

    packet[8] = 1;

    outsw(
        ATA_DATA,
        packet,
        6
    );

    if(!wait_drq())
    {
        print("READ10 failed\n");
        return false;
    }

    insw(
        ATA_DATA,
        buffer,
        1024
    );

    return true;
}
