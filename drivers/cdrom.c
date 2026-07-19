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
#define ATA_CMD_IDENTIFY 0xEC
#define ATAPI_READ10    0x28


#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08
#define STATUS_ERR 0x01


static uint8_t cd_drive = 0;


static bool wait_busy_clear()
{
    int timeout = 1000000;

    while(timeout--)
    {
        uint8_t status = inb(ATA_STATUS);

        if(!(status & STATUS_BSY))
            return true;
    }

    return false;
}



static bool wait_drq()
{
    int timeout = 1000000;

    while(timeout--)
    {
        uint8_t status = inb(ATA_STATUS);


        if(status & STATUS_ERR)
            return false;


        if(status & STATUS_DRQ)
            return true;
    }


    return false;
}




static bool check_drive(uint8_t drive)
{
    uint8_t lba1;
    uint8_t lba2;


    outb(
        ATA_HDDEVSEL,
        drive
    );


    io_wait();


    outb(
        ATA_COMMAND,
        ATA_CMD_IDENTIFY
    );


    io_wait();


    uint8_t status = inb(ATA_STATUS);


    if(status == 0)
        return false;



    if(!wait_busy_clear())
        return false;



    lba1 = inb(ATA_LBA1);
    lba2 = inb(ATA_LBA2);



    /*
        ATAPI signature
    */

    if((lba1 == 0x14 && lba2 == 0xEB) ||
       (lba1 == 0x69 && lba2 == 0x96))
    {
        return true;
    }


    return false;
}





void cdrom_init(void)
{
    print("Searching CD-ROM...\n");


    if(check_drive(0xA0))
    {
        cd_drive = 0xA0;
        print("CD-ROM master found\n");
        return;
    }



    if(check_drive(0xB0))
    {
        cd_drive = 0xB0;
        print("CD-ROM slave found\n");
        return;
    }



    print("No ATAPI CD-ROM found\n");
}






bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(cd_drive == 0)
    {
        print("No CD drive\n");
        return false;
    }



    outb(
        ATA_HDDEVSEL,
        cd_drive
    );


    io_wait();



    if(!wait_busy_clear())
        return false;



    /*
        Set transfer size = 2048 bytes
    */

    outb(
        ATA_FEATURES,
        0
    );


    outb(
        ATA_LBA1,
        0x00
    );


    outb(
        ATA_LBA2,
        0x08
    );



    outb(
        ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq())
    {
        print("PACKET DRQ failed\n");
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
        print("READ10 DRQ failed\n");
        return false;
    }



    insw(
        ATA_DATA,
        buffer,
        1024
    );


    return true;
}
