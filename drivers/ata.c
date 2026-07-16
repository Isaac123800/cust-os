#include "ata.h"


static uint32_t total_sectors = 0;


/*
    Wait until ATA drive is ready

    Returns false on timeout/error
*/

bool ata_wait(void)
{
    uint8_t status;

    int timeout = 1000000;


    while((status = inb(ATA_PRIMARY_STATUS)) & ATA_SR_BSY)
    {
        timeout--;

        if(timeout <= 0)
            return false;
    }


    if(status & ATA_SR_ERR)
        return false;


    if(status & ATA_SR_DF)
        return false;



    timeout = 1000000;


    while(!(status = inb(ATA_PRIMARY_STATUS)) & ATA_SR_DRQ)
    {
        timeout--;

        if(timeout <= 0)
            return false;


        if(status & ATA_SR_ERR)
            return false;
    }


    return true;
}





/*
    Detect ATA drive

    Reads IDENTIFY information
*/

bool ata_detect(void)
{
    uint8_t status;


    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER
    );


    io_wait();



    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_IDENTIFY
    );



    status =
        inb(ATA_PRIMARY_STATUS);



    if(status == 0)
        return false;



    while(1)
    {
        status =
            inb(ATA_PRIMARY_STATUS);


        if(status & ATA_SR_ERR)
            return false;


        if(status & ATA_SR_DRQ)
            break;
    }



    uint16_t buffer[256];


    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );



    /*
        IDENTIFY words:

        word 60 = lower 16 bits
        word 61 = upper 16 bits

        LBA28 sector count
    */

    total_sectors =
        ((uint32_t)buffer[61] << 16)
        | buffer[60];



    return true;
}





/*
    Initialize ATA driver
*/

void ata_init(void)
{
    ata_detect();
}





/*
    Read one sector
*/

bool ata_read_sector(
    uint32_t lba,
    uint8_t *buffer
)
{

    if(!ata_wait())
        return false;



    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER |
        ((lba >> 24) & 0x0F)
    );


    io_wait();



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



    if(!ata_wait())
        return false;



    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );


    return true;
}





/*
    Write one sector
*/

bool ata_write_sector(
    uint32_t lba,
    const uint8_t *buffer
)
{

    if(!ata_wait())
        return false;



    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER |
        ((lba >> 24) & 0x0F)
    );


    io_wait();



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





/*
    Flush ATA cache
*/

void ata_flush(void)
{
    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_CACHE_FLUSH
    );


    ata_wait();
}





/*
    Return detected disk size
*/

uint32_t ata_sector_count(void)
{
    return total_sectors;
}
