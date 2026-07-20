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



#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1



#define ATAPI_READ10              0x28



#define STATUS_ERR   0x01
#define STATUS_DRQ   0x08
#define STATUS_DF    0x20
#define STATUS_DRDY  0x40
#define STATUS_BSY   0x80



static uint16_t cd_base = 0;
static uint8_t cd_drive = 0;
static bool cd_found = false;




static void print_hex(uint8_t value)
{
    char hex[] =
        "0123456789ABCDEF";


    char out[3];


    out[0] =
        hex[(value >> 4) & 0xF];


    out[1] =
        hex[value & 0xF];


    out[2] = 0;


    print(out);
}





static void ata_delay(uint16_t base)
{
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
}
/*
    Wait until the drive is no longer busy
*/

static bool wait_not_busy(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            inb(base + ATA_STATUS);


        if(!(status & STATUS_BSY))
        {
            return true;
        }
    }


    print("ATAPI BUSY TIMEOUT\n");

    return false;
}





/*
    Wait until drive requests data
*/

static bool wait_drq(uint16_t base)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            inb(base + ATA_STATUS);



        if(status & STATUS_ERR)
        {
            print("ATAPI ERROR\n");
            return false;
        }



        if(status & STATUS_DF)
        {
            print("ATAPI FAULT\n");
            return false;
        }



        if(!(status & STATUS_BSY) &&
           (status & STATUS_DRQ))
        {
            return true;
        }
    }



    print("ATAPI DRQ TIMEOUT\n");

    return false;
}








static bool detect_atapi(
    uint16_t base,
    uint8_t drive
)
{
    print("Checking IDE\n");



    /*
        Select drive

        0x40 enables LBA mode
    */

    outb(
        base + ATA_HDDEVSEL,
        drive | 0x40
    );


    ata_delay(base);



    /*
        Clear registers before IDENTIFY PACKET
    */

    outb(base + ATA_FEATURES, 0);
    outb(base + ATA_SECCOUNT0, 0);
    outb(base + ATA_LBA0, 0);
    outb(base + ATA_LBA1, 0);
    outb(base + ATA_LBA2, 0);



    ata_delay(base);



    uint8_t status =
        inb(base + ATA_STATUS);



    print("STATUS ");

    print_hex(status);

    print("\n");



    /*
        No device
    */

    if(status == 0)
    {
        return false;
    }



    /*
        Wait for device ready
    */

    if(!wait_not_busy(base))
    {
        return false;
    }



    /*
        ATAPI signature is stored
        in LBA1 and LBA2
    */

    uint8_t sig1 =
        inb(base + ATA_LBA1);


    uint8_t sig2 =
        inb(base + ATA_LBA2);



    print("SIG ");

    print_hex(sig1);

    print(" ");

    print_hex(sig2);

    print("\n");



    if(!((sig1 == 0x14 && sig2 == 0xEB) ||
         (sig1 == 0x69 && sig2 == 0x96)))
    {
        return false;
    }



    print("ATAPI SIGNATURE OK\n");



    outb(
        base + ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );



    if(!wait_drq(base))
    {
        print("IDENTIFY PACKET FAILED\n");
        return false;
    }



    uint16_t identify[256];


    insw(
        base + ATA_DATA,
        identify,
        256
    );



    print("IDENTIFY OK\n");



    cd_base = base;
    cd_drive = drive;
    cd_found = true;



    print("ATAPI CD FOUND\n");


    return true;
}
void cdrom_init(void)
{
    print("CD INIT\n");



    cd_found = false;
    cd_base = 0;
    cd_drive = 0;



    /*
        QEMU layout:

        Secondary IDE Master = CD-ROM
        Primary IDE Master   = HDD
    */

    uint16_t bases[2] =
    {
        0x170,
        0x1F0
    };



    uint8_t drives[2] =
    {
        0xA0,
        0xB0
    };



    for(int b = 0;
        b < 2;
        b++)
    {
        for(int d = 0;
            d < 2;
            d++)
        {

            print("PORT ");


            print_hex(
                (bases[b] >> 8) & 0xFF
            );


            print_hex(
                bases[b] & 0xFF
            );



            print(" DRIVE ");



            if(drives[d] == 0xA0)
            {
                print("MASTER\n");
            }
            else
            {
                print("SLAVE\n");
            }



            if(detect_atapi(
                bases[b],
                drives[d]))
            {
                print("CD-ROM INITIALIZED\n");

                return;
            }
        }
    }



    print("NO ATAPI CD-ROM FOUND\n");
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



    /*
        Select CD-ROM drive
    */

    outb(
        cd_base + ATA_HDDEVSEL,
        cd_drive | 0x40
    );


    ata_delay(cd_base);



    if(!wait_not_busy(cd_base))
    {
        return false;
    }



    /*
        Set transfer size.

        ATAPI uses LBA1/LBA2
        to specify byte count.

        0x0800 = 2048 bytes
    */

    outb(
        cd_base + ATA_FEATURES,
        0
    );


    outb(
        cd_base + ATA_LBA1,
        0x00
    );


    outb(
        cd_base + ATA_LBA2,
        0x08
    );



    /*
        Send PACKET command
    */

    outb(
        cd_base + ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq(cd_base))
    {
        print("PACKET DRQ FAILED\n");
        return false;
    }




    /*
        SCSI READ(10) packet
    */

    uint8_t packet[12];


    for(int i = 0;
        i < 12;
        i++)
    {
        packet[i] = 0;
    }



    packet[0] =
        ATAPI_READ10;



    packet[2] =
        (sector >> 24) & 0xFF;


    packet[3] =
        (sector >> 16) & 0xFF;


    packet[4] =
        (sector >> 8) & 0xFF;


    packet[5] =
        sector & 0xFF;



    /*
        Transfer 1 sector
    */

    packet[8] = 1;



    /*
        12 bytes = 6 words
    */

    outsw(
        cd_base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(cd_base))
    {
        print("READ DATA DRQ FAILED\n");
        return false;
    }




    uint16_t size =
        inb(cd_base + ATA_LBA1)
        |
        (inb(cd_base + ATA_LBA2) << 8);



    if(size == 0)
    {
        print("CD RETURNED ZERO SIZE\n");
        return false;
    }



    if(size > 2048)
    {
        print("CD RETURNED TOO MUCH DATA\n");
        return false;
    }



    insw(
        cd_base + ATA_DATA,
        buffer,
        size / 2
    );



    if(!wait_not_busy(cd_base))
    {
        return false;
    }



    uint8_t status =
        inb(cd_base + ATA_STATUS);



    if(status & STATUS_ERR)
    {
        print("CD READ ERROR\n");
        return false;
    }



    return true;
}
