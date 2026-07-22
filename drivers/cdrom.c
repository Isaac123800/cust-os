#include "cdrom.h"

#include "../include/types.h"
#include "../io.h"


extern void print(char *text);


// IDE ports

#define PRIMARY_IO      0x1F0
#define SECONDARY_IO    0x170


// ATA registers

#define ATA_DEVICE      6
#define ATA_STATUS      7
#define ATA_COMMAND     7


// Commands

#define ATA_IDENTIFY    0xEC


static void ide_delay()
{
    inb(0x80);
    inb(0x80);
    inb(0x80);
    inb(0x80);
}



static bool ide_detect(
    uint16_t io,
    uint8_t drive
)
{
    uint8_t status;


    // Select drive
    outb(io + ATA_DEVICE, 0xA0 | (drive << 4));

    ide_delay();


    // Send IDENTIFY command
    outb(io + ATA_COMMAND, ATA_IDENTIFY);


    status = inb(io + ATA_STATUS);


    // No device
    if(status == 0)
    {
        return false;
    }


    // Wait for busy flag to clear
    while(status & 0x80)
    {
        status = inb(io + ATA_STATUS);
    }


    // Error flag
    if(status & 0x01)
    {
        return false;
    }


    return true;
}



void cdrom_init(void)
{
    print("IDE DEVICE SCAN\n");


    if(ide_detect(PRIMARY_IO,0))
    {
        print("Primary Master detected\n");
    }
    else
    {
        print("Primary Master empty\n");
    }


    if(ide_detect(PRIMARY_IO,1))
    {
        print("Primary Slave detected\n");
    }
    else
    {
        print("Primary Slave empty\n");
    }


    if(ide_detect(SECONDARY_IO,0))
    {
        print("Secondary Master detected\n");
    }
    else
    {
        print("Secondary Master empty\n");
    }


    if(ide_detect(SECONDARY_IO,1))
    {
        print("Secondary Slave detected\n");
    }
    else
    {
        print("Secondary Slave empty\n");
    }


    print("IDE SCAN COMPLETE\n");
}



bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    // Not implemented yet.
    // This will be added after ATAPI detection works.

    (void)sector;
    (void)buffer;


    return false;
}
