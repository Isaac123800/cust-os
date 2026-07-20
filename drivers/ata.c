#include "ata.h"

extern void print(char *text);


static uint32_t total_sectors = 0;
static bool ata_ready = false;



static void print_hex(uint8_t value)
{
    char hex[] = "0123456789ABCDEF";

    char out[3];

    out[0] = hex[(value >> 4) & 0xF];
    out[1] = hex[value & 0xF];
    out[2] = 0;

    print(out);
}



static void ata_delay(void)
{
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
}



/*
    Wait until drive is ready
*/

bool ata_wait(void)
{
    int timeout = 1000000;

    uint8_t status;


    while(timeout--)
    {
        status = inb(ATA_PRIMARY_STATUS);

        if(!(status & ATA_SR_BSY))
            break;
    }


    if(timeout <= 0)
    {
        print("ATA BUSY TIMEOUT\n");
        return false;
    }



    if(status & ATA_SR_ERR)
    {
        print("ATA ERROR\n");
        return false;
    }



    if(status & ATA_SR_DF)
    {
        print("ATA DEVICE FAULT\n");
        return false;
    }



    timeout = 1000000;


    while(timeout--)
    {
        status = inb(ATA_PRIMARY_STATUS);


        if(status & ATA_SR_ERR)
        {
            print("ATA ERROR WAITING DRQ\n");
            return false;
        }


        if(status & ATA_SR_DF)
        {
            print("ATA FAULT WAITING DRQ\n");
            return false;
        }


        if(status & ATA_SR_DRQ)
            return true;
    }


    print("ATA DRQ TIMEOUT\n");

    return false;
}





bool ata_detect(void)
{
    print("ATA DETECT\n");


    outb(
        ATA_PRIMARY_HDDEVSEL,
        0xE0
    );


    ata_delay();


    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_IDENTIFY
    );


    ata_delay();


    uint8_t status =
        inb(ATA_PRIMARY_STATUS);


    print("ATA STATUS ");

    print_hex(status);

    print("\n");


    if(status == 0)
    {
        print("NO ATA DEVICE\n");
        return false;
    }


    if(!ata_wait())
    {
        print("IDENTIFY FAILED\n");
        return false;
    }



    uint16_t buffer[256];


    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );



    total_sectors =
        ((uint32_t)buffer[61] << 16)
        |
        buffer[60];


    print("ATA DEVICE OK\n");


    return true;
}





void ata_init(void)
{
    print("ATA INIT\n");


    ata_ready = ata_detect();


    if(!ata_ready)
    {
        print("ATA INIT FAILED\n");
    }
}







bool ata_read_sector(
    uint32_t lba,
    uint8_t *buffer
)
{

    if(!ata_ready)
    {
        print("ATA NOT READY\n");
        return false;
    }



    outb(
        ATA_PRIMARY_HDDEVSEL,
        0xE0 | ((lba >> 24) & 0x0F)
    );


    ata_delay();



    outb(
        ATA_PRIMARY_SECCOUNT0,
        1
    );


    outb(
        ATA_PRIMARY_LBA0,
        (uint8_t)lba
    );


    outb(
        ATA_PRIMARY_LBA1,
        (uint8_t)(lba >> 8)
    );


    outb(
        ATA_PRIMARY_LBA2,
        (uint8_t)(lba >> 16)
    );



    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_READ_SECTORS
    );


    ata_delay();



    if(!ata_wait())
        return false;



    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );


    return true;
}







bool ata_write_sector(
    uint32_t lba,
    const uint8_t *buffer
)
{

    if(!ata_ready)
    {
        print("ATA NOT READY\n");
        return false;
    }



    outb(
        ATA_PRIMARY_HDDEVSEL,
        0xE0 | ((lba >> 24) & 0x0F)
    );


    ata_delay();



    outb(
        ATA_PRIMARY_SECCOUNT0,
        1
    );


    outb(
        ATA_PRIMARY_LBA0,
        (uint8_t)lba
    );


    outb(
        ATA_PRIMARY_LBA1,
        (uint8_t)(lba >> 8)
    );


    outb(
        ATA_PRIMARY_LBA2,
        (uint8_t)(lba >> 16)
    );



    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_WRITE_SECTORS
    );


    ata_delay();



    if(!ata_wait())
        return false;



    outsw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );


    ata_flush();


    return true;
}







void ata_flush(void)
{

    if(!ata_ready)
        return;


    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_CACHE_FLUSH
    );


    ata_delay();

    ata_wait();
}







uint32_t ata_sector_count(void)
{
    return total_sectors;
}
