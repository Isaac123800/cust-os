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



static void print_hex(uint8_t n)
{
    char hex[] = "0123456789ABCDEF";

    char out[3];

    out[0] = hex[(n >> 4) & 0xF];
    out[1] = hex[n & 0xF];
    out[2] = 0;

    print(out);
}






static bool wait_not_busy()
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status = inb(ATA_STATUS);


        if(!(status & STATUS_BSY))
            return true;
    }


    print("DEBUG: BUSY TIMEOUT\n");

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
            print("DEBUG: STATUS ERROR\n");
            return false;
        }


        if(status & STATUS_DRQ)
            return true;
    }


    print("DEBUG: DRQ TIMEOUT\n");

    return false;
}









static void check_device(
    uint16_t base,
    uint8_t drive
)
{

    print("DEBUG: Checking base ");

    print_hex(base >> 8);
    print_hex(base & 0xFF);

    print(" drive ");

    print_hex(drive);

    print("\n");



    outb(
        ATA_HDDEVSEL,
        drive
    );


    io_wait();



    uint8_t status =
        inb(ATA_STATUS);



    print("DEBUG: STATUS ");

    print_hex(status);

    print("\n");



    if(status == 0)
    {
        print("DEBUG: No device\n");
        return;
    }



    if(!wait_not_busy())
    {
        print("DEBUG: Device busy\n");
        return;
    }




    uint8_t lba1 =
        inb(ATA_LBA1);

    uint8_t lba2 =
        inb(ATA_LBA2);



    print("DEBUG: LBA1 ");

    print_hex(lba1);

    print(" LBA2 ");

    print_hex(lba2);

    print("\n");



    if(
       lba1 == 0x14 &&
       lba2 == 0xEB
      )
    {

        print("DEBUG: ATAPI SIGNATURE FOUND\n");

        cd_found = true;

        return;
    }



    if(
       lba1 == 0x69 &&
       lba2 == 0x96
      )
    {
        print("DEBUG: ATAPI SIGNATURE FOUND (ALT)\n");

        cd_found = true;

        return;
    }



    print("DEBUG: Not ATAPI\n");
}








void cdrom_init(void)
{
    print("DEBUG: CD INIT START\n");


    // Primary IDE
    check_device(
        0x1F0,
        0xA0
    );


    check_device(
        0x1F0,
        0xB0
    );



    // Secondary IDE
    check_device(
        0x170,
        0xA0
    );


    check_device(
        0x170,
        0xB0
    );



    if(cd_found)
    {
        print("DEBUG: CD READY\n");
    }
    else
    {
        print("DEBUG: NO CD FOUND\n");
    }
}









bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    print("DEBUG: READ SECTOR\n");

    if(!cd_found)
    {
        print("DEBUG: READ FAILED - NO CD\n");
        return false;
    }



    print("DEBUG: Selecting CD\n");


    outb(
        ATA_HDDEVSEL,
        0xA0
    );


    io_wait();



    if(!wait_not_busy())
        return false;



    print("DEBUG: Sending PACKET\n");



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


    outb(
        ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq())
    {
        print("DEBUG: PACKET DRQ FAILED\n");
        return false;
    }



    print("DEBUG: Sending READ10\n");



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



    print("DEBUG: Waiting DATA\n");



    if(!wait_drq())
    {
        print("DEBUG: DATA DRQ FAILED\n");
        return false;
    }



    print("DEBUG: Reading 2048 bytes\n");



    insw(
        ATA_DATA,
        buffer,
        1024
    );



    print("DEBUG: SECTOR OK\n");


    return true;
}
