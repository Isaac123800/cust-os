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


#define ATA_CMD_PACKET           0xA0
#define ATA_CMD_IDENTIFY_PACKET  0xA1

#define ATAPI_READ10             0x28


#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_BSY 0x80



static uint16_t cd_base = 0;
static uint8_t cd_drive = 0;
static bool cd_found = false;




static void write_reg(
    uint16_t base,
    uint8_t reg,
    uint8_t value
)
{
    outb(base + reg, value);
}




static uint8_t read_reg(
    uint16_t base,
    uint8_t reg
)
{
    return inb(base + reg);
}





static bool wait_not_busy(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            read_reg(base, ATA_STATUS);


        if(status & STATUS_ERR)
            return false;


        if(!(status & STATUS_BSY))
            return true;
    }


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
            return false;


        if(!(status & STATUS_BSY) &&
           (status & STATUS_DRQ))
        {
            return true;
        }
    }


    return false;
}







static bool detect_device(
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



    // Clear registers
    write_reg(base, ATA_FEATURES, 0);
    write_reg(base, ATA_SECCOUNT0, 0);
    write_reg(base, ATA_LBA0, 0);
    write_reg(base, ATA_LBA1, 0);
    write_reg(base, ATA_LBA2, 0);



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



    if(!wait_not_busy(base))
        return false;



    uint8_t lba1 =
        read_reg(base, ATA_LBA1);


    uint8_t lba2 =
        read_reg(base, ATA_LBA2);



    if(
       (lba1 == 0x14 && lba2 == 0xEB) ||
       (lba1 == 0x69 && lba2 == 0x96)
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
    print("Searching CD-ROM\n");



    uint16_t channels[] =
    {
        0x1F0,
        0x170
    };



    for(int i = 0; i < 2; i++)
    {

        if(detect_device(
            channels[i],
            0xA0))
        {
            print("CD-ROM found\n");
            return;
        }



        if(detect_device(
            channels[i],
            0xB0))
        {
            print("CD-ROM found\n");
            return;
        }
    }



    print("NO CD\n");
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



    uint16_t base = cd_base;



    write_reg(
        base,
        ATA_HDDEVSEL,
        cd_drive
    );


    io_wait();



    if(!wait_not_busy(base))
        return false;




    // Request 2048 byte transfer

    write_reg(
        base,
        ATA_FEATURES,
        0
    );


    write_reg(
        base,
        ATA_LBA1,
        0
    );


    write_reg(
        base,
        ATA_LBA2,
        8
    );



    // Send PACKET command

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
