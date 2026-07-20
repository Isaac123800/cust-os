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
    Wait until drive stops being busy
*/

static bool ata_wait_not_busy(void)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            inb(ATA_PRIMARY_STATUS);


        if(!(status & ATA_SR_BSY))
        {
            return true;
        }
    }


    print("ATA BUSY TIMEOUT\n");

    return false;
}





/*
    Wait until data transfer is ready
*/

static bool ata_wait_drq(void)
{
    int timeout = 1000000;


    while(timeout--)
    {
        uint8_t status =
            inb(ATA_PRIMARY_STATUS);



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



        if((status & ATA_SR_DRQ) &&
           !(status & ATA_SR_BSY))
        {
            return true;
        }
    }



    print("ATA DRQ TIMEOUT\n");

    return false;
}







bool ata_detect(void)
{
    print("ATA DETECT\n");


    /*
        Select primary master
        LBA mode
    */

    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER
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



    if(!ata_wait_not_busy())
    {
        return false;
    }



    uint16_t buffer[256];



    if(!(inb(ATA_PRIMARY_STATUS) & ATA_SR_DRQ))
    {
        print("NO IDENTIFY DATA\n");
        return false;
    }



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
        ATA_MASTER | ((lba >> 24) & 0x0F)
    );


    ata_delay();



    if(!ata_wait_not_busy())
    {
        return false;
    }



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



    if(!ata_wait_drq())
    {
        return false;
    }



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
        ATA_MASTER | ((lba >> 24) & 0x0F)
    );


    ata_delay();



    if(!ata_wait_not_busy())
    {
        return false;
    }



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



    if(!ata_wait_drq())
    {
        return false;
    }



    outsw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );



    if(!ata_wait_not_busy())
    {
        return false;
    }



    ata_flush();



    return true;
}








void ata_flush(void)
{
    if(!ata_ready)
    {
        return;
    }



    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_CACHE_FLUSH
    );



    ata_wait_not_busy();
}







uint32_t ata_sector_count(void)
{
    return total_sectors;
}
