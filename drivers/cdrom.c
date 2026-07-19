#include "cdrom.h"
#include "io.h"

extern void print(char *text);


#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_FEATURES    1
#define ATA_SECCOUNT0   2
#define ATA_LBA0        3
#define ATA_LBA1        4
#define ATA_LBA2        5
#define ATA_HDDEVSEL    6
#define ATA_COMMAND     7
#define ATA_STATUS      7


#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1

#define ATAPI_READ10            0x28


#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_BSY 0x80



static uint16_t cd_base = 0;
static uint8_t cd_drive = 0;
static bool cd_found = false;



static uint8_t read_reg(uint16_t base, uint8_t reg)
{
    return inb(base + reg);
}



static void write_reg(uint16_t base, uint8_t reg, uint8_t value)
{
    outb(base + reg, value);
}




static bool wait_ready(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            read_reg(base, ATA_STATUS);


        if(status & STATUS_ERR)
        {
            print("ATA ERROR\n");
            return false;
        }


        if(!(status & STATUS_BSY))
            return true;
    }


    print("READY TIMEOUT\n");
    return false;
}





static bool wait_drq(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            read_reg(base, ATA_STATUS);


        if(status & STATUS_ERR)
        {
            print("ATAPI ERROR\n");
            return false;
        }


        if(!(status & STATUS_BSY) &&
           (status & STATUS_DRQ))
        {
            return true;
        }
    }


    print("DRQ TIMEOUT\n");
    return false;
}





static bool detect(
    uint16_t base,
    uint8_t drive
)
{

    write_reg(
        base,
        ATA_HDDEVSEL,
        drive
    );


    io_wait();


    write_reg(
        base,
        ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );


    io_wait();



    uint8_t status =
        read_reg(base, ATA_STATUS);



    if(status == 0)
        return false;



    if(!wait_ready(base))
        return false;



    uint8_t sig1 =
        read_reg(base, ATA_LBA1);


    uint8_t sig2 =
        read_reg(base, ATA_LBA2);



    if(
       (sig1 == 0x14 && sig2 == 0xEB) ||
       (sig1 == 0x69 && sig2 == 0x96)
      )
    {

        cd_base = base;
        cd_drive = drive;
        cd_found = true;

        return true;
    }


    return false;
}







void cdrom_init(void)
{
    print("Searching CD-ROM...\n");


    uint16_t bases[] =
    {
        0x1F0,
        0x170
    };



    for(int i = 0; i < 2; i++)
    {

        if(detect(
            bases[i],
            0xA0))
        {
            print("ATAPI CD found\n");
            return;
        }



        if(detect(
            bases[i],
            0xB0))
        {
            print("ATAPI CD found\n");
            return;
        }
    }



    print("No CD-ROM\n");
}








bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(!cd_found)
    {
        print("No CD\n");
        return false;
    }



    print("Reading sector\n");



    uint16_t base = cd_base;



    write_reg(
        base,
        ATA_HDDEVSEL,
        cd_drive
    );


    io_wait();



    if(!wait_ready(base))
        return false;




    /*
        Set transfer size:
        2048 bytes
    */


    write_reg(
        base,
        ATA_FEATURES,
        0
    );


    write_reg(
        base,
        ATA_LBA1,
        0x00
    );


    write_reg(
        base,
        ATA_LBA2,
        0x08
    );




    /*
        PACKET command
    */


    write_reg(
        base,
        ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq(base))
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
        base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(base))
    {
        print("DATA DRQ FAIL\n");
        return false;
    }



    insw(
        base + ATA_DATA,
        buffer,
        1024
    );



    return true;
}
