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


#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08
#define STATUS_ERR 0x01


static bool cd_found = false;


static bool wait_not_busy()
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status = inb(ATA_STATUS);


        if(!(status & STATUS_BSY))
            return true;
    }


    print("BUSY TIMEOUT\n");
    return false;
}




static bool wait_drq()
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status = inb(ATA_STATUS);


        if(status & STATUS_ERR)
        {
            print("ATA ERROR\n");
            return false;
        }


        if(status & STATUS_DRQ)
            return true;
    }


    print("DRQ TIMEOUT\n");
    return false;
}







void cdrom_init(void)
{
    print("Checking CD-ROM\n");


    // Secondary IDE master (QEMU default CD)
    outb(
        ATA_HDDEVSEL,
        0xA0
    );


    io_wait();


    if(!wait_not_busy())
        return;



    uint8_t sig1 = inb(ATA_LBA1);
    uint8_t sig2 = inb(ATA_LBA2);



    if(sig1 == 0x14 &&
       sig2 == 0xEB)
    {
        print("ATAPI CD FOUND\n");

        cd_found = true;
        return;
    }



    print("NO ATAPI SIGNATURE\n");
}








bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(!cd_found)
    {
        print("NO CD\n");
        return false;
    }



    outb(
        ATA_HDDEVSEL,
        0xA0
    );


    io_wait();



    if(!wait_not_busy())
        return false;




    // Request 2048 bytes

    outb(
        ATA_FEATURES,
        0
    );


    outb(
        ATA_LBA1,
        0
    );


    outb(
        ATA_LBA2,
        8
    );



    // PACKET

    outb(
        ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq())
    {
        print("PACKET DRQ FAIL\n");
        return false;
    }




    uint8_t packet[12];


    for(int i = 0; i < 12; i++)
        packet[i] = 0;



    packet[0] = ATAPI_READ10;



    packet[2] =
        (sector >> 24) & 0xFF;

    packet[3] =
        (sector >> 16) & 0xFF;

    packet[4] =
        (sector >> 8) & 0xFF;

    packet[5] =
        sector & 0xFF;



    packet[8] = 1;



    outsw(
        ATA_DATA,
        packet,
        6
    );



    if(!wait_drq())
    {
        print("READ DRQ FAIL\n");
        return false;
    }



    insw(
        ATA_DATA,
        buffer,
        1024
    );


    return true;
}
