#include "cdrom.h"

#include "io.h"


#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_FEATURES    0x1F1
#define ATA_PRIMARY_SECCOUNT0   0x1F2
#define ATA_PRIMARY_LBA0        0x1F3
#define ATA_PRIMARY_LBA1        0x1F4
#define ATA_PRIMARY_LBA2        0x1F5
#define ATA_PRIMARY_HDDEVSEL    0x1F6
#define ATA_PRIMARY_COMMAND     0x1F7
#define ATA_PRIMARY_STATUS      0x1F7


#define ATA_SR_BSY  0x80
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01



#define ATAPI_CMD_PACKET 0xA0
#define ATAPI_CMD_READ10 0x28



static bool wait_ready()
{
    int timeout = 1000000;


    uint8_t status;


    while((status = inb(ATA_PRIMARY_STATUS)) & ATA_SR_BSY)
    {
        timeout--;

        if(timeout <= 0)
            return false;
    }


    while(!(status & ATA_SR_DRQ))
    {
        status = inb(ATA_PRIMARY_STATUS);


        if(status & ATA_SR_ERR)
            return false;


        timeout--;


        if(timeout <= 0)
            return false;
    }


    return true;
}





void cdrom_init(void)
{

}





bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(!wait_ready())
        return false;



    /*
        Select ATAPI master device
    */

    outb(
        ATA_PRIMARY_HDDEVSEL,
        0xA0
    );


    io_wait();



    /*
        Set transfer size
        2048 bytes
    */

    outb(
        ATA_PRIMARY_FEATURES,
        0
    );


    outb(
        ATA_PRIMARY_LBA1,
        0x08
    );


    outb(
        ATA_PRIMARY_LBA2,
        0
    );



    /*
        Send PACKET command
    */

    outb(
        ATA_PRIMARY_COMMAND,
        ATAPI_CMD_PACKET
    );



    if(!wait_ready())
        return false;



    /*
        READ(10) packet
    */

    uint8_t packet[12];


    for(int i = 0; i < 12; i++)
        packet[i] = 0;



    packet[0] = ATAPI_CMD_READ10;


    packet[2] =
        (sector >> 24) & 0xFF;

    packet[3] =
        (sector >> 16) & 0xFF;

    packet[4] =
        (sector >> 8) & 0xFF;

    packet[5] =
        sector & 0xFF;


    packet[7] = 0;

    packet[8] = 1;



    /*
        Send packet
    */

    outsw(
        ATA_PRIMARY_DATA,
        packet,
        6
    );



    if(!wait_ready())
        return false;



    /*
        Read 2048 bytes
    */

    insw(
        ATA_PRIMARY_DATA,
        buffer,
        1024
    );



    return true;
}
