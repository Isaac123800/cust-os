#include "cdrom.h"
#include "io.h"

extern void print(char *text);


#define ATA_DATA        0x00
#define ATA_ERROR       0x01
#define ATA_FEATURES    0x01
#define ATA_SECCOUNT0   0x02
#define ATA_LBA0        0x03
#define ATA_LBA1        0x04
#define ATA_LBA2        0x05
#define ATA_HDDEVSEL    0x06
#define ATA_COMMAND     0x07
#define ATA_STATUS      0x07


#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1


#define ATAPI_READ10 0x28


#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_BSY 0x80



static uint16_t cd_base = 0;
static uint8_t cd_drive = 0;
static bool cd_found = false;



static void print_hex(uint8_t value)
{
    char hex[] = "0123456789ABCDEF";

    char out[3];

    out[0] = hex[(value >> 4) & 0xF];
    out[1] = hex[value & 0xF];
    out[2] = 0;

    print(out);
}





static bool wait_not_busy(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            inb(base + ATA_STATUS);


        if(!(status & STATUS_BSY))
            return true;
    }


    print("BUSY TIMEOUT\n");

    return false;
}





static bool wait_drq(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            inb(base + ATA_STATUS);


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







static bool detect_atapi(
    uint16_t base,
    uint8_t drive
)
{

    print("Checking IDE\n");


    outb(
        base + ATA_HDDEVSEL,
        drive
    );


    io_wait();



    outb(base + ATA_FEATURES, 0);
    outb(base + ATA_SECCOUNT0, 0);
    outb(base + ATA_LBA0, 0);
    outb(base + ATA_LBA1, 0);
    outb(base + ATA_LBA2, 0);



    outb(
        base + ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );


    io_wait();



    uint8_t status =
        inb(base + ATA_STATUS);



    print("STATUS ");

    print_hex(status);

    print("\n");



    if(status == 0)
        return false;



    if(!wait_not_busy(base))
        return false;



    uint8_t lba1 =
        inb(base + ATA_LBA1);


    uint8_t lba2 =
        inb(base + ATA_LBA2);



    print("SIG ");

    print_hex(lba1);

    print(" ");

    print_hex(lba2);

    print("\n");



    if(
       (lba1 == 0x14 && lba2 == 0xEB) ||
       (lba1 == 0x69 && lba2 == 0x96)
      )
    {

        print("ATAPI CD FOUND\n");


        cd_base = base;
        cd_drive = drive;
        cd_found = true;


        return true;
    }


    return false;
}








void cdrom_init(void)
{

    print("CD INIT\n");


    uint16_t bases[] =
    {
        0x1F0,
        0x170
    };


    uint8_t drives[] =
    {
        0xA0,
        0xB0
    };



    for(int b = 0; b < 2; b++)
    {
        for(int d = 0; d < 2; d++)
        {

            if(detect_atapi(
                bases[b],
                drives[d]))
            {
                return;
            }
        }
    }


    print("NO CD FOUND\n");
}








bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(!cd_found)
    {
        print("READ: NO CD\n");
        return false;
    }



    outb(
        cd_base + ATA_HDDEVSEL,
        cd_drive
    );


    io_wait();



    if(!wait_not_busy(cd_base))
        return false;




    // 2048 byte transfer

    outb(
        cd_base + ATA_FEATURES,
        0
    );


    outb(
        cd_base + ATA_LBA1,
        0
    );


    outb(
        cd_base + ATA_LBA2,
        8
    );



    outb(
        cd_base + ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq(cd_base))
    {
        print("PACKET DRQ FAIL\n");
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
        cd_base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(cd_base))
    {
        print("DATA DRQ FAIL\n");
        return false;
    }



    insw(
        cd_base + ATA_DATA,
        buffer,
        1024
    );


    return true;
}
