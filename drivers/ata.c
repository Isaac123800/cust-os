#include "ata.h"

#include "../include/console.h"


// ============================================================
// ATA STATE
// ============================================================

static uint32_t total_sectors = 0;

static bool ata_ready = false;


// ============================================================
// PRINT HEX
// ============================================================

static void print_hex_local(
    uint8_t value
)
{
    char hex[] =
        "0123456789ABCDEF";

    char out[3];

    out[0] =
        hex[(value >> 4) & 0x0F];

    out[1] =
        hex[value & 0x0F];

    out[2] = 0;

    print(out);
}


// ============================================================
// ATA DELAY
// ============================================================

static void ata_delay(void)
{
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
    inb(ATA_PRIMARY_ALTSTATUS);
}


// ============================================================
// SELECT PRIMARY MASTER
// ============================================================

static void ata_select_drive(
    uint32_t lba
)
{
    outb(
        ATA_PRIMARY_HDDEVSEL,
        ATA_MASTER |
        ((lba >> 24) & 0x0F)
    );

    ata_delay();
}


// ============================================================
// WAIT UNTIL NOT BUSY
// ============================================================

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

    print(
        "ATA BUSY TIMEOUT\n"
    );

    return false;
}


// ============================================================
// WAIT FOR DRQ
// ============================================================

static bool ata_wait_drq(void)
{
    uint32_t timeout = 1000000;

    while(timeout--)
    {
        uint8_t status =
            inb(ATA_PRIMARY_STATUS);

        if(status & ATA_SR_ERR)
        {
            print(
                "ATA ERROR STATUS "
            );

            print_hex_local(status);

            print("\n");

            return false;
        }

        if(status & ATA_SR_DF)
        {
            print(
                "ATA DEVICE FAULT STATUS "
            );

            print_hex_local(status);

            print("\n");

            return false;
        }

        if((status & ATA_SR_DRQ) &&
           !(status & ATA_SR_BSY))
        {
            return true;
        }
    }

    print(
        "ATA DRQ TIMEOUT\n"
    );

    return false;
}


// ============================================================
// WAIT FOR WRITE COMPLETION
// ============================================================

static bool ata_wait_write_complete(void)
{
    uint32_t timeout = 1000000;

    while(timeout--)
    {
        uint8_t status =
            inb(ATA_PRIMARY_STATUS);

        if(status & ATA_SR_ERR)
        {
            print(
                "ATA WRITE ERROR STATUS "
            );

            print_hex_local(status);

            print("\n");

            return false;
        }

        if(status & ATA_SR_DF)
        {
            print(
                "ATA WRITE DEVICE FAULT "
            );

            print_hex_local(status);

            print("\n");

            return false;
        }

        /*
            Write is complete when:

            BSY = 0
            DRQ = 0
        */

        if(!(status & ATA_SR_BSY) &&
           !(status & ATA_SR_DRQ))
        {
            return true;
        }
    }

    print(
        "ATA WRITE COMPLETE TIMEOUT\n"
    );

    print(
        "FINAL STATUS "
    );

    print_hex_local(
        inb(ATA_PRIMARY_STATUS)
    );

    print("\n");

    return false;
}


// ============================================================
// CHECK ATA STATUS
// ============================================================

static bool ata_check_status(void)
{
    uint8_t status =
        inb(ATA_PRIMARY_STATUS);

    if(status & ATA_SR_ERR)
    {
        print(
            "ATA COMMAND ERROR STATUS "
        );

        print_hex_local(status);

        print("\n");

        return false;
    }

    if(status & ATA_SR_DF)
    {
        print(
            "ATA DEVICE FAULT STATUS "
        );

        print_hex_local(status);

        print("\n");

        return false;
    }

    return true;
}


// ============================================================
// DETECT ATA DISK
// ============================================================

bool ata_detect(void)
{
    print(
        "ATA DETECT\n"
    );

    /*
        Select Primary Master.
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

    print(
        "ATA STATUS "
    );

    print_hex_local(status);

    print("\n");


    if(status == 0)
    {
        print(
            "NO ATA DEVICE\n"
        );

        return false;
    }


    /*
        Wait for device.
    */

    if(!ata_wait_not_busy())
    {
        return false;
    }


    status =
        inb(ATA_PRIMARY_STATUS);


    if(status & ATA_SR_ERR)
    {
        print(
            "ATA IDENTIFY ERROR\n"
        );

        return false;
    }


    if(!(status & ATA_SR_DRQ))
    {
        print(
            "NO IDENTIFY DATA\n"
        );

        return false;
    }


    /*
        Read the 512-byte IDENTIFY block.
    */

    uint16_t buffer[256];

    insw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );


    /*
        Words 60-61 contain the number
        of user-addressable LBA28 sectors.
    */

    total_sectors =
        ((uint32_t)buffer[61] << 16) |
        buffer[60];


    print(
        "ATA DEVICE OK\n"
    );

    print(
        "ATA SECTORS "
    );

    print_hex_local(
        (uint8_t)(total_sectors >> 24)
    );

    print_hex_local(
        (uint8_t)(total_sectors >> 16)
    );

    print_hex_local(
        (uint8_t)(total_sectors >> 8)
    );

    print_hex_local(
        (uint8_t)total_sectors
    );

    print("\n");

    return true;
}


// ============================================================
// INITIALIZE ATA
// ============================================================

void ata_init(void)
{
    print(
        "ATA INIT\n"
    );

    ata_ready =
        ata_detect();

    if(!ata_ready)
    {
        print(
            "ATA INIT FAILED\n"
        );
    }
}


// ============================================================
// READ ONE SECTOR
// ============================================================

bool ata_read_sector(
    uint32_t lba,
    uint8_t *buffer
)
{
    if(!ata_ready)
    {
        print(
            "ATA NOT READY\n"
        );

        return false;
    }


    print(
        "ATA READ LBA "
    );

    print_hex_local(
        (uint8_t)(lba >> 24)
    );

    print_hex_local(
        (uint8_t)(lba >> 16)
    );

    print_hex_local(
        (uint8_t)(lba >> 8)
    );

    print_hex_local(
        (uint8_t)lba
    );

    print("\n");


    /*
        Select disk.
    */

    ata_select_drive(lba);


    print(
        "READ SELECT STATUS "
    );

    print_hex_local(
        inb(ATA_PRIMARY_STATUS)
    );

    print("\n");


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
        LBA28 address.
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

    ata_delay();


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


    print(
        "READ DATA "
    );

    print_hex_local(buffer[0]);
    print(" ");

    print_hex_local(buffer[1]);
    print(" ");

    print_hex_local(buffer[2]);
    print(" ");

    print_hex_local(buffer[3]);

    print("\n");


    return ata_check_status();
}


// ============================================================
// WRITE ONE SECTOR
// ============================================================

bool ata_write_sector(
    uint32_t lba,
    const uint8_t *buffer
)
{
    if(!ata_ready)
    {
        print(
            "ATA NOT READY\n"
        );

        return false;
    }


    print(
        "ATA WRITE LBA "
    );

    print_hex_local(
        (uint8_t)(lba >> 24)
    );

    print_hex_local(
        (uint8_t)(lba >> 16)
    );

    print_hex_local(
        (uint8_t)(lba >> 8)
    );

    print_hex_local(
        (uint8_t)lba
    );

    print("\n");


    /*
        Show sector contents.
    */

    print(
        "WRITE DATA "
    );

    print_hex_local(
        buffer[0]
    );

    print(" ");

    print_hex_local(
        buffer[1]
    );

    print(" ");

    print_hex_local(
        buffer[2]
    );

    print(" ");

    print_hex_local(
        buffer[3]
    );

    print(" ... ");

    print_hex_local(
        buffer[510]
    );

    print(" ");

    print_hex_local(
        buffer[511]
    );

    print("\n");


    /*
        Select disk.
    */

    ata_select_drive(lba);


    print(
        "WRITE SELECT STATUS "
    );

    print_hex_local(
        inb(ATA_PRIMARY_STATUS)
    );

    print("\n");


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
        LBA28 address.
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
        WRITE SECTORS.
    */

    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_WRITE_SECTORS
    );

    ata_delay();


    if(!ata_wait_drq())
    {
        return false;
    }


    /*
        Send 512 bytes.
    */

    outsw(
        ATA_PRIMARY_DATA,
        buffer,
        256
    );


    print(
        "AFTER DATA STATUS "
    );

    print_hex_local(
        inb(ATA_PRIMARY_STATUS)
    );

    print("\n");


    /*
        Wait until the device has finished.
    */

    if(!ata_wait_write_complete())
    {
        return false;
    }


    print(
        "WRITE COMPLETE STATUS "
    );

    print_hex_local(
        inb(ATA_PRIMARY_STATUS)
    );

    print("\n");


    if(!ata_check_status())
    {
        return false;
    }


    /*
        Flush the drive cache.
    */

    ata_flush();


    print(
        "ATA WRITE OK\n"
    );

    return true;
}


// ============================================================
// FLUSH CACHE
// ============================================================

void ata_flush(void)
{
    if(!ata_ready)
    {
        return;
    }


    print(
        "ATA FLUSH\n"
    );


    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_CACHE_FLUSH
    );

    ata_delay();


    if(!ata_wait_not_busy())
    {
        print(
            "ATA FLUSH TIMEOUT\n"
        );

        return;
    }


    if(!ata_check_status())
    {
        return;
    }


    print(
        "ATA FLUSH OK\n"
    );
}


// ============================================================
// SECTOR COUNT
// ============================================================

uint32_t ata_sector_count(void)
{
    return total_sectors;
}

