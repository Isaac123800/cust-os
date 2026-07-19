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


#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08
#define STATUS_ERR 0x01



typedef struct
{
    uint16_t base;
    uint8_t drive;
    bool found;

} CDDevice;



static CDDevice cdrom;




static void out_port(uint16_t base, uint8_t reg, uint8_t value)
{
    outb(base + reg, value);
}


static uint8_t in_port(uint16_t base, uint8_t reg)
{
    return inb(base + reg);
}





static bool wait_not_busy(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            in_port(base, ATA_STATUS);


        if(status & STATUS_ERR)
            return false;


        if(!(status & STATUS_BSY))
            return true;
    }


    return false;
}







static bool detect_device(
    uint16_t base,
    uint8_t drive
)
{
    out_port(
        base,
        ATA_HDDEVSEL,
        drive
    );


    io_wait();



    out_port(
        base,
        ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );


    io_wait();



    uint8_t status =
        in_port(base, ATA_STATUS);



    if(status == 0)
        return false;



    if(!wait_not_busy(base))
        return false;



    uint8_t sig1 =
        in_port(base, ATA_LBA1);


    uint8_t sig2 =
        in_port(base, ATA_LBA2);



    if(
        (sig1 == 0x14 && sig2 == 0xEB) ||
        (sig1 == 0x69 && sig2 == 0x96)
      )
    {
        cdrom.base = base;
        cdrom.drive = drive;
        cdrom.found = true;

        return true;
    }


    return false;
}






void cdrom_init(void)
{
    print("Searching ATAPI CD-ROM...\n");


    cdrom.found = false;



    uint16_t bases[] =
    {
        0x1F0,
        0x170
    };


    for(int b = 0; b < 2; b++)
    {
        if(detect_device(
            bases[b],
            0xA0))
        {
            print("CD-ROM found\n");
            return;
        }



        if(detect_device(
            bases[b],
            0xB0))
        {
            print("CD-ROM found\n");
            return;
        }
    }



    print("No ATAPI CD-ROM\n");
}







static bool wait_drq(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            in_port(base, ATA_STATUS);


        if(status & STATUS_ERR)
            return false;



        if(status & STATUS_DRQ)
            return true;
    }


    return false;
}








bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(!cdrom.found)
        return false;



    uint16_t base =
        cdrom.base;



    out_port(
        base,
        ATA_HDDEVSEL,
        cdrom.drive
    );


    io_wait();



    if(!wait_not_busy(base))
        return false;



    /*
        2048 byte transfer
    */

    out_port(base, ATA_FEATURES, 0);
    out_port(base, ATA_LBA1, 0x00);
    out_port(base, ATA_LBA2, 0x08);



    out_port(
        base,
        ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq(base))
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


    packet[8] = 0;
    packet[9] = 1;



    outsw(
        base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(base))
    {
        print("READ DRQ failed\n");
        return false;
    }



    insw(
        base + ATA_DATA,
        buffer,
        1024
    );


    return true;
}
