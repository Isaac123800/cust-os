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





static void ata_write(
    uint16_t base,
    uint8_t reg,
    uint8_t value
)
{
    outb(base + reg, value);
}




static uint8_t ata_read(
    uint16_t base,
    uint8_t reg
)
{
    return inb(base + reg);
}






static void select_drive(
    uint16_t base,
    uint8_t drive
)
{
    ata_write(
        base,
        ATA_HDDEVSEL,
        drive
    );

    io_wait();
}







static bool wait_busy_clear(
    uint16_t base
)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            ata_read(base, ATA_STATUS);


        if(!(status & STATUS_BSY))
            return true;
    }


    return false;
}






static bool wait_drq(
    uint16_t base
)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            ata_read(base, ATA_STATUS);


        if(status & STATUS_ERR)
            return false;


        if(status & STATUS_DRQ)
            return true;
    }


    return false;
}









static bool detect_atapi(
    uint16_t base,
    uint8_t drive
)
{

    select_drive(
        base,
        drive
    );


    // Reset registers

    ata_write(base, ATA_FEATURES, 0);
    ata_write(base, ATA_SECCOUNT0, 0);
    ata_write(base, ATA_LBA0, 0);
    ata_write(base, ATA_LBA1, 0);
    ata_write(base, ATA_LBA2, 0);



    ata_write(
        base,
        ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );


    io_wait();



    uint8_t status =
        ata_read(
            base,
            ATA_STATUS
        );



    if(status == 0)
        return false;



    // Wait for response

    if(!wait_busy_clear(base))
        return false;



    uint8_t lba1 =
        ata_read(base, ATA_LBA1);


    uint8_t lba2 =
        ata_read(base, ATA_LBA2);



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


    uint16_t channels[2];

    channels[0] = 0x1F0; // primary
    channels[1] = 0x170; // secondary



    for(int c = 0; c < 2; c++)
    {

        if(detect_atapi(
            channels[c],
            0xA0))
        {
            print("ATAPI CD found\n");
            return;
        }



        if(detect_atapi(
            channels[c],
            0xB0))
        {
            print("ATAPI CD found\n");
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



    select_drive(
        base,
        cd_drive
    );



    if(!wait_busy_clear(base))
    {
        print("BUSY FAIL\n");
        return false;
    }



    /*
       Request 2048 byte transfer
    */

    ata_write(
        base,
        ATA_FEATURES,
        0
    );


    ata_write(
        base,
        ATA_LBA1,
        0x00
    );


    ata_write(
        base,
        ATA_LBA2,
        0x08
    );




    /*
       Send ATAPI PACKET
    */

    ata_write(
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



    // Read one sector

    packet[8] = 1;



    outsw(
        base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(base))
    {
        print("READ DRQ FAIL\n");
        return false;
    }



    insw(
        base + ATA_DATA,
        buffer,
        1024
    );



    return true;
}
