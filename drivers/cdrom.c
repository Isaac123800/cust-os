#include "cdrom.h"

#include "io.h"


#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_FEATURES    0x1F1
#define ATA_PRIMARY_SECCOUNT0   0x1F2
#define ATA_PRIMARY_LBA0        0x1F3
#define ATA_PRIMARY_LBA1        0x1F4
#define ATA_PRIMARY_LBA2        0x1F5
#define ATA_PRIMARY_HDDEVSEL    0x1F6
#define ATA_PRIMARY_COMMAND     0x1F7
#define ATA_PRIMARY_STATUS      0x1F7


#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1


#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01



static bool cdrom_found = false;



static bool cdrom_wait()
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


    return true;
}





void cdrom_init(void)
{
    /*
        ATAPI detection will go here.

        For now we mark it as unavailable.
    */


    cdrom_found = false;
}






bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    if(!cdrom_found)
        return false;


    /*
        ATAPI READ(10) packet command
        will be added here.
    */


    return false;
}
