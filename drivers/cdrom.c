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
#define ATAPI_REQUEST_SENSE       0x03




#define STATUS_ERR  0x01
#define STATUS_DRQ  0x08
#define STATUS_DF   0x20
#define STATUS_BSY  0x80




#define ATAPI_SIG1 0x14
#define ATAPI_SIG2 0xEB

#define ATAPI_SIG3 0x69
#define ATAPI_SIG4 0x96






static uint16_t cd_base = 0;
static uint8_t  cd_drive = 0;
static bool     cd_found = false;







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
    /*
        ATA 400ns delay
    */

    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
}







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



        if(status & STATUS_DF)
        {
            print("ATA DEVICE FAULT\n");
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
static bool detect_atapi(
    uint16_t base,
    uint8_t drive
)
{
    print("Checking IDE\n");



    /*
        Select device

        Keep this as drive only.
        Do not add 0x40 here.
    */

    outb(
        base + ATA_HDDEVSEL,
        drive
    );


    ata_delay(base);



    /*
        Clear registers
    */

    outb(base + ATA_FEATURES, 0);
    outb(base + ATA_SECCOUNT0, 0);
    outb(base + ATA_LBA0, 0);
    outb(base + ATA_LBA1, 0);
    outb(base + ATA_LBA2, 0);



    if(!wait_not_busy(base))
    {
        return false;
    }





    /*
        Check ATAPI signature
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



    if(!((sig1 == ATAPI_SIG1 &&
          sig2 == ATAPI_SIG2)
          ||
         (sig1 == ATAPI_SIG3 &&
          sig2 == ATAPI_SIG4)))
    {
        return false;
    }



    print("ATAPI SIGNATURE OK\n");





    /*
        Send IDENTIFY PACKET
    */

    outb(
        base + ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );



    if(!wait_drq(base))
    {
        print("IDENTIFY FAILED\n");
        return false;
    }





    /*
        Read IDENTIFY data

        256 words = 512 bytes
    */

    uint16_t identify[256];


    insw(
        base + ATA_DATA,
        identify,
        256
    );



    print("ATAPI DEVICE FOUND\n");



    cd_base = base;

    cd_drive = drive;

    cd_found = true;



    return true;
}
void cdrom_init(void)
{
    print("CD INIT\n");



    cd_found = false;
    cd_base = 0;
    cd_drive = 0;



    /*
        IDE channels:

        0x170 = Secondary
        0x1F0 = Primary

        Usually:
        Secondary Master = CD-ROM
    */


    uint16_t ports[] =
    {
        0x170,
        0x1F0
    };



    uint8_t drives[] =
    {
        0xA0,
        0xB0
    };





    for(int p = 0;
        p < 2;
        p++)
    {
        for(int d = 0;
            d < 2;
            d++)
        {

            print("CHECK ");

            print_hex(
                (ports[p] >> 8) & 0xFF
            );


            print_hex(
                ports[p] & 0xFF
            );


            print(" ");



            if(drives[d] == 0xA0)
            {
                print("MASTER\n");
            }
            else
            {
                print("SLAVE\n");
            }





            if(detect_atapi(
                ports[p],
                drives[d]))
            {
                print("CD-ROM INITIALIZED\n");
                return;
            }
        }
    }



    print("NO ATAPI CD-ROM FOUND\n");
}
static bool cdrom_request_sense(void)
{
    if(!cd_found)
    {
        return false;
    }



    outb(
        cd_base + ATA_HDDEVSEL,
        cd_drive
    );


    ata_delay(cd_base);



    if(!wait_not_busy(cd_base))
    {
        return false;
    }





    /*
        Request sense data

        Allocation length = 18 bytes
    */


    outb(
        cd_base + ATA_FEATURES,
        0
    );


    outb(
        cd_base + ATA_LBA1,
        18
    );


    outb(
        cd_base + ATA_LBA2,
        0
    );



    outb(
        cd_base + ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq(cd_base))
    {
        return false;
    }





    uint8_t packet[12] =
    {
        0
    };


    packet[0] = ATAPI_REQUEST_SENSE;

    packet[4] = 18;



    outsw(
        cd_base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(cd_base))
    {
        return false;
    }





    uint8_t sense[18];


    insw(
        cd_base + ATA_DATA,
        sense,
        9
    );



    print("SENSE ");

    print_hex(
        sense[2] & 0x0F
    );

    print("\n");



    return true;
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
        Select CD-ROM
    */

    outb(
        cd_base + ATA_HDDEVSEL,
        cd_drive
    );


    ata_delay(cd_base);



    if(!wait_not_busy(cd_base))
    {
        return false;
    }





    /*
        Request 2048 byte transfer
    */

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



    /*
        Start ATAPI PACKET command
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



    print("PACKET READY\n");





    /*
        SCSI READ(10)
    */

    uint8_t packet[12] =
    {
        0
    };


    packet[0] = ATAPI_READ10;


    packet[2] = (sector >> 24) & 0xFF;

    packet[3] = (sector >> 16) & 0xFF;

    packet[4] = (sector >> 8) & 0xFF;

    packet[5] = sector & 0xFF;



    /*
        Read one sector
    */

    packet[8] = 1;



    outsw(
        cd_base + ATA_DATA,
        packet,
        6
    );



    print("READ PACKET SENT\n");



    if(!wait_drq(cd_base))
    {
        print("READ DRQ FAILED\n");

        cdrom_request_sense();

        return false;
    }



    print("READ DATA READY\n");
    /*
        Read transfer size from ATA registers

        LBA1/LBA2 contain the byte count
    */

    uint16_t size =
        inb(cd_base + ATA_LBA1)
        |
        (inb(cd_base + ATA_LBA2) << 8);



    if(size == 0)
    {
        print("ZERO DATA SIZE\n");

        cdrom_request_sense();

        return false;
    }



    /*
        ATAPI CD sectors are normally 2048 bytes

        Prevent buffer overflow
    */

    if(size > 2048)
    {
        print("DATA TOO LARGE\n");

        cdrom_request_sense();

        return false;
    }





    /*
        Transfer words

        insw count is WORDS,
        not bytes
    */

    insw(
        cd_base + ATA_DATA,
        buffer,
        size / 2
    );





    /*
        Wait for command completion
    */

    if(!wait_not_busy(cd_base))
    {
        return false;
    }





    uint8_t status =
        inb(cd_base + ATA_STATUS);



    if(status & STATUS_ERR)
    {
        print("READ ERROR\n");

        cdrom_request_sense();

        return false;
    }





    /*
        Clear unused part of the buffer

        Keeps caller data predictable
    */

    for(uint16_t i = size;
        i < 2048;
        i++)
    {
        buffer[i] = 0;
    }





    print("SECTOR READ OK\n");


    return true;
}
