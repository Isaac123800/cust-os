#include "ata.h"


/*
    Wait until ATA drive is ready

    Returns false if an error occurs
*/

bool ata_wait(void)
{
    uint8_t status;


    /*
        Wait for BSY to clear
    */

    while((status = inb(ATA_PRIMARY_STATUS)) & ATA_SR_BSY)
    {

    }



    /*
        Check errors
    */

    if(status & ATA_SR_ERR)
        return false;


    if(status & ATA_SR_DF)
        return false;



    /*
        Wait for data request

    */

    while(!(status = inb(ATA_PRIMARY_STATUS) & ATA_SR_DRQ))
    {
        if(status & ATA_SR_ERR)
            return false;
    }


    return true;
}





/*
    Detect ATA drive

*/

bool ata_detect(void)
{
    uint8_t status;



    /*
        Select master drive
    */

    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER
    );


    io_wait();



    /*
        Send IDENTIFY command
    */

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_IDENTIFY
    );



    status =
        inb(ATA_PRIMARY_STATUS);



    /*
        No drive connected
    */

    if(status == 0)
        return false;



    /*
        Wait for response
    */

    while(1)
    {
        status =
            inb(ATA_PRIMARY_STATUS);


        if(status & ATA_SR_ERR)
            return false;


        if(status & ATA_SR_DRQ)
            break;
    }



    /*
        Read identify data

        We don't use it yet,
        but the drive responded.
    */

    uint16_t buffer[256];


    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );


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

    LBA28 addressing

*/

bool ata_read_sector(
    uint32_t lba,
    uint8_t *buffer
)
{

    if(!ata_wait())
        return false;



    /*
        Select drive and high LBA bits
    */

    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER |
        ((lba >> 24) & 0x0F)
    );



    io_wait();



    /*
        Number of sectors

        We read one sector
    */

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



    /*
        Send read command
    */

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_READ_SECTORS
    );



    if(!ata_wait())
        return false;



    /*
        Read 256 words = 512 bytes
    */

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



    /*
        Select drive
    */

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



    /*
        Send write command
    */

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_WRITE_SECTORS
    );



    if(!ata_wait())
        return false;



    /*
        Write 512 bytes

        256 words
    */

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
