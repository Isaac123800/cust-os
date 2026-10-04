#include "ata.h"

extern void print(char *text);


/*
    ============================================================
    ATA STATE
    ============================================================
*/

static uint32_t total_sectors = 0;
static bool ata_ready = false;


/*
    ============================================================
    DEBUG HEX
    ============================================================
*/

static void print_hex(uint8_t value)
{
    char hex[] = "0123456789ABCDEF";
    char out[3];

    out[0] = hex[(value >> 4) & 0xF];
    out[1] = hex[value & 0xF];
    out[2] = 0;

    print(out);
}


/*
    ============================================================
    ATA DELAY
    ============================================================
*/

static void ata_delay(void)
{
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
}


/*
    ============================================================
    SELECT PRIMARY MASTER
    ============================================================
*/

static void ata_select_drive(uint32_t lba)
{
    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER | ((lba >> 24) & 0x0F)
    );

    ata_delay();
}


/*
    ============================================================
    WAIT UNTIL NOT BUSY
    ============================================================
*/

static bool ata_wait_not_busy(void)
{
    uint32_t timeout = 1000000;

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
    ============================================================
    WAIT FOR DRQ
    ============================================================
*/

static bool ata_wait_drq(void)
{
    uint32_t timeout = 1000000;

    while(timeout--)
    {
        uint8_t status =
            inb(ATA_PRIMARY_STATUS);

        if(status & ATA_SR_ERR)
        {
            print("ATA ERROR\n");

            print("STATUS ");
            print_hex(status);
            print("\n");

            return false;
        }

        if(status & ATA_SR_DF)
        {
            print("ATA DEVICE FAULT\n");

            print("STATUS ");
            print_hex(status);
            print("\n");

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


/*
    ============================================================
    CHECK FINAL STATUS
    ============================================================
*/

static bool ata_check_status(void)
{
    uint8_t status =
        inb(ATA_PRIMARY_STATUS);

    if(status & ATA_SR_ERR)
    {
        print("ATA COMMAND ERROR\n");

        print("STATUS ");
        print_hex(status);
        print("\n");

        return false;
    }

    if(status & ATA_SR_DF)
    {
        print("ATA DEVICE FAULT\n");

        print("STATUS ");
        print_hex(status);
        print("\n");

        return false;
    }

    return true;
}


/*
    ============================================================
    IDENTIFY DRIVE
    ============================================================
*/

bool ata_detect(void)
{
    print("ATA DETECT\n");

    /*
        Select primary master.
    */

    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER
    );

    ata_delay();

    /*
        Clear task-file registers.
    */

    outb(
        ATA_PRIMARY_SECCOUNT0,
        0
    );

    outb(
        ATA_PRIMARY_LBA0,
        0
    );

    outb(
        ATA_PRIMARY_LBA1,
        0
    );

    outb(
        ATA_PRIMARY_LBA2,
        0
    );

    /*
        Send IDENTIFY.
    */

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

    /*
        Wait until the drive is ready.
    */

    if(!ata_wait_not_busy())
    {
        return false;
    }

    status =
        inb(ATA_PRIMARY_STATUS);

    if(status & ATA_SR_ERR)
    {
        print("ATA IDENTIFY ERROR\n");
        return false;
    }

    if(!(status & ATA_SR_DRQ))
    {
        print("NO IDENTIFY DATA\n");
        return false;
    }

    /*
        Read IDENTIFY data.
    */

    uint16_t buffer[256];

    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );

    /*
        Words 60-61 contain the
        28-bit sector count.
    */

    total_sectors =
        ((uint32_t)buffer[61] << 16)
        |
        buffer[60];

    print("ATA DEVICE OK\n");

    return true;
}


/*
    ============================================================
    INITIALISE ATA
    ============================================================
*/

void ata_init(void)
{
    print("ATA INIT\n");

    ata_ready = ata_detect();

    if(!ata_ready)
    {
        print("ATA INIT FAILED\n");
    }
}


/*
    ============================================================
    READ ONE SECTOR
    ============================================================
*/

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

    /*
        Select primary master using LBA28.
    */

    ata_select_drive(lba);

    /*
        Wait until the drive is ready.
    */

    if(!ata_wait_not_busy())
    {
        return false;
    }

    /*
        Sector count = 1.
    */

    outb(
        ATA_PRIMARY_SECCOUNT0,
        1
    );

    /*
        LBA address.
    */

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
        READ SECTORS.
    */

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_READ_SECTORS
    );

    /*
        400 ns delay.
    */

    ata_delay();

    /*
        Wait for data.
    */

    if(!ata_wait_drq())
    {
        return false;
    }

    /*
        Read 512 bytes = 256 words.
    */

    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );

    /*
        Check final status.
    */

    return ata_check_status();
}


/*
    ============================================================
    WRITE ONE SECTOR
    ============================================================
*/

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

    print("ATA WRITE LBA ");

    print_hex((uint8_t)lba);

    print("\n");

    /*
        Select primary master using LBA28.
    */

    ata_select_drive(lba);

    /*
        Wait until the drive is ready.
    */

    if(!ata_wait_not_busy())
    {
        return false;
    }

    /*

