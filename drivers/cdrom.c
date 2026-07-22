#include "cdrom.h"

#include "../include/types.h"
#include "io.h"


extern void print(char *text);


// IDE ports

#define PRIMARY_IO      0x1F0
#define SECONDARY_IO    0x170


// ATA registers

#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_SECTOR      2
#define ATA_LBA_LOW     3
#define ATA_LBA_MID     4
#define ATA_LBA_HIGH    5
#define ATA_DEVICE      6
#define ATA_STATUS      7
#define ATA_COMMAND     7

#define ATA_FEATURES        1
#define ATA_BYTE_COUNT_LOW  4
#define ATA_BYTE_COUNT_HIGH 5


// Commands

#define ATA_IDENTIFY        0xEC
#define ATA_PACKET          0xA0

#define ATAPI_READ10        0x28



static void ide_delay()
{
    inb(0x80);
    inb(0x80);
    inb(0x80);
    inb(0x80);
}



static bool wait_not_busy(uint16_t io)
{
    int timeout = 100000;

    while(timeout--)
    {
        uint8_t status = inb(io + ATA_STATUS);

        if(!(status & 0x80))
        {
            return true;
        }
    }

    return false;
}



static bool wait_drq(uint16_t io)
{
    int timeout = 100000;

    while(timeout--)
    {
        uint8_t status = inb(io + ATA_STATUS);


        if(status & 0x01)
        {
            return false;
        }


        if(status & 0x08)
        {
            return true;
        }
    }

    return false;
}



static uint8_t ide_identify(
    uint16_t io,
    uint8_t drive
)
{
    uint8_t status;
    uint8_t lba_mid;
    uint8_t lba_high;


    outb(io + ATA_DEVICE, 0xA0 | (drive << 4));

    ide_delay();


    outb(io + ATA_SECTOR, 0);
    outb(io + ATA_LBA_LOW, 0);
    outb(io + ATA_LBA_MID, 0);
    outb(io + ATA_LBA_HIGH, 0);


    outb(io + ATA_COMMAND, ATA_IDENTIFY);


    status = inb(io + ATA_STATUS);


    if(status == 0)
        return 0;



    while(status & 0x80)
    {
        status = inb(io + ATA_STATUS);
    }



    lba_mid = inb(io + ATA_LBA_MID);
    lba_high = inb(io + ATA_LBA_HIGH);



    if(lba_mid == 0x14 && lba_high == 0xEB)
    {
        return 2;
    }



    if(status & 0x01)
    {
        return 0;
    }


    return 1;
}



static void check_device(
    uint16_t io,
    uint8_t drive,
    char *name
)
{
    uint8_t result = ide_identify(io, drive);


    print(name);


    if(result == 0)
        print(": Empty\n");

    else if(result == 1)
        print(": ATA Hard Disk\n");

    else if(result == 2)
        print(": ATAPI CD-ROM\n");
}



void cdrom_init(void)
{
    print("IDE DEVICE IDENTIFY\n");


    check_device(PRIMARY_IO,0,"Primary Master");
    check_device(PRIMARY_IO,1,"Primary Slave");
    check_device(SECONDARY_IO,0,"Secondary Master");
    check_device(SECONDARY_IO,1,"Secondary Slave");


    print("IDENTIFY COMPLETE\n");
}





bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    uint16_t io = SECONDARY_IO;

    uint8_t packet[12];


    print("ATAPI: START\n");


    outb(io + ATA_DEVICE,0xA0);

    ide_delay();



    if(!wait_not_busy(io))
    {
        print("ATAPI: BUSY TIMEOUT\n");
        return false;
    }



    print("ATAPI: READY\n");



    // 2048 byte transfer

    outb(io + ATA_FEATURES,0);

    outb(io + ATA_BYTE_COUNT_LOW,0x00);
    outb(io + ATA_BYTE_COUNT_HIGH,0x08);



    outb(io + ATA_COMMAND,ATA_PACKET);


    print("ATAPI: PACKET SENT\n");



    if(!wait_drq(io))
    {
        print("ATAPI: NO DRQ\n");
        return false;
    }



    print("ATAPI: DRQ OK\n");



    for(int i=0;i<12;i++)
        packet[i]=0;



    // READ(10)

    packet[0] = ATAPI_READ10;



    // LBA

    packet[2] = (sector >> 24) & 0xFF;
    packet[3] = (sector >> 16) & 0xFF;
    packet[4] = (sector >> 8) & 0xFF;
    packet[5] = sector & 0xFF;



    // Transfer 1 sector

    packet[7] = 0;
    packet[8] = 1;



    outsw(
        io + ATA_DATA,
        packet,
        6
    );


    print("ATAPI: READ COMMAND SENT\n");



    if(!wait_drq(io))
    {
        print("ATAPI: READ ERROR\n");
        return false;
    }



    print("ATAPI: READING DATA\n");



    insw(
        io + ATA_DATA,
        buffer,
        CD_SECTOR_SIZE / 2
    );



    print("ATAPI: READ OK\n");


    return true;
}
