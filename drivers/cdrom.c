#include "cdrom.h"

#include "../include/types.h"
#include "io.h"

extern void print(char *text);
extern void print_hex(uint8_t value);


// ============================================================
// IDE PORTS
// ============================================================

#define PRIMARY_IO      0x1F0
#define SECONDARY_IO    0x170


// ============================================================
// ATA REGISTERS
// ============================================================

#define ATA_DATA                0
#define ATA_ERROR               1
#define ATA_FEATURES            1

#define ATA_INTERRUPT_REASON    2
#define ATA_SECTOR_COUNT        2

#define ATA_LBA_LOW             3
#define ATA_LBA_MID             4
#define ATA_LBA_HIGH            5

#define ATA_BYTE_COUNT_LOW      4
#define ATA_BYTE_COUNT_HIGH     5

#define ATA_DEVICE              6

#define ATA_STATUS              7
#define ATA_COMMAND             7


// ============================================================
// ATA / ATAPI COMMANDS
// ============================================================

#define ATA_IDENTIFY            0xEC
#define ATA_IDENTIFY_PACKET     0xA1
#define ATA_PACKET              0xA0


// ============================================================
// STATUS BITS
// ============================================================

#define ATA_ERR     0x01
#define ATA_DRQ     0x08
#define ATA_DF      0x20
#define ATA_DRDY    0x40
#define ATA_BSY     0x80


// ============================================================
// CD-ROM STATE
// ============================================================

static uint16_t cdrom_io = 0;
static uint8_t cdrom_drive = 0;


// ============================================================
// IDE DELAY
// ============================================================

static void ide_delay(void)
{
    io_wait();
    io_wait();
    io_wait();
    io_wait();
}


// ============================================================
// WAIT UNTIL DEVICE IS NOT BUSY
// ============================================================

static bool wait_not_busy(uint16_t io)
{
    int timeout = 5000000;

    while(timeout--)
    {
        uint8_t status =
            inb(io + ATA_STATUS);

        if(!(status & ATA_BSY))
        {
            return true;
        }
    }

    return false;
}


// ============================================================
// WAIT FOR DATA REQUEST
// ============================================================

static bool wait_drq(uint16_t io)
{
    int timeout = 5000000;

    while(timeout--)
    {
        uint8_t status =
            inb(io + ATA_STATUS);

        if(status & ATA_ERR)
        {
            return false;
        }

        if(status & ATA_DF)
        {
            return false;
        }

        if(!(status & ATA_BSY) &&
           (status & ATA_DRQ))
        {
            return true;
        }
    }

    return false;
}


// ============================================================
// DEBUG STATUS
// ============================================================

static void dump_status(uint16_t io)
{
    uint8_t status =
        inb(io + ATA_STATUS);

    uint8_t error =
        inb(io + ATA_ERROR);

    uint8_t reason =
        inb(io + ATA_INTERRUPT_REASON);

    print("STATUS=");
    print_hex(status);

    print(" ERROR=");
    print_hex(error);

    print(" REASON=");
    print_hex(reason);

    print("\n");
}


// ============================================================
// IDENTIFY DEVICE
//
// Return:
//
// 0 = no device
// 1 = ATA device
// 2 = ATAPI device
// ============================================================

static uint8_t ide_identify(
    uint16_t io,
    uint8_t drive
)
{
    uint8_t device =
        0xA0 | (drive << 4);

    // --------------------------------------------------------
    // Select drive
    // --------------------------------------------------------

    outb(
        io + ATA_DEVICE,
        device
    );

    ide_delay();

    // --------------------------------------------------------
    // Clear task-file registers
    // --------------------------------------------------------

    outb(
        io + ATA_SECTOR_COUNT,
        0
    );

    outb(
        io + ATA_LBA_LOW,
        0
    );

    outb(
        io + ATA_LBA_MID,
        0
    );

    outb(
        io + ATA_LBA_HIGH,
        0
    );

    // --------------------------------------------------------
    // Check whether device/channel exists
    // --------------------------------------------------------

    uint8_t status =
        inb(io + ATA_STATUS);

    if(status == 0xFF)
    {
        return 0;
    }

    if(status == 0)
    {
        return 0;
    }

    // --------------------------------------------------------
    // Send ATA IDENTIFY
    // --------------------------------------------------------

    outb(
        io + ATA_COMMAND,
        ATA_IDENTIFY
    );

    ide_delay();

    // --------------------------------------------------------
    // Wait for device
    // --------------------------------------------------------

    if(!wait_not_busy(io))
    {
        return 0;
    }

    status =
        inb(io + ATA_STATUS);

    // --------------------------------------------------------
    // IDENTIFY failed.
    //
    // Check whether this is an ATAPI device.
    // --------------------------------------------------------

    if(status & ATA_ERR)
    {
        uint8_t mid =
            inb(io + ATA_LBA_MID);

        uint8_t high =
            inb(io + ATA_LBA_HIGH);

        if(!(
            (mid == 0x14 && high == 0xEB) ||
            (mid == 0x69 && high == 0x96)
        ))
        {
            return 0;
        }

        // ----------------------------------------------------
        // ATAPI device found.
        //
        // Use IDENTIFY PACKET DEVICE and consume its data.
        // ----------------------------------------------------

        outb(
            io + ATA_DEVICE,
            device
        );

        ide_delay();

        outb(
            io + ATA_SECTOR_COUNT,
            0
        );

        outb(
            io + ATA_LBA_LOW,
            0
        );

        outb(
            io + ATA_LBA_MID,
            0
        );

        outb(
            io + ATA_LBA_HIGH,
            0
        );

        outb(
            io + ATA_COMMAND,
            ATA_IDENTIFY_PACKET
        );

        ide_delay();

        if(!wait_not_busy(io))
        {
            return 0;
        }

        status =
            inb(io + ATA_STATUS);

        if(status & ATA_ERR)
        {
            return 0;
        }

        if(!(status & ATA_DRQ))
        {
            return 0;
        }

        uint16_t identify_buffer[256];

        insw(
            io + ATA_DATA,
            identify_buffer,
            256
        );

        return 2;
    }

    // --------------------------------------------------------
    // Normal ATA device
    //
    // IDENTIFY returned data.
    //
    // IMPORTANT:
    // We MUST consume the 512-byte identification block.
    // Otherwise DRQ remains set on the device.
    // --------------------------------------------------------

    if(!(status & ATA_DRQ))
    {
        return 0;
    }

    uint16_t identify_buffer[256];

    insw(
        io + ATA_DATA,
        identify_buffer,
        256
    );

    return 1;
}


// ============================================================
// CHECK DEVICE
// ============================================================

static void check_device(
    uint16_t io,
    uint8_t drive,
    char *name
)
{
    uint8_t result =
        ide_identify(
            io,
            drive
        );

    print(name);

    if(result == 0)
    {
        print(": Empty\n");
    }
    else if(result == 1)
    {
        print(": ATA Disk\n");
    }
    else if(result == 2)
    {
        print(": ATAPI CD-ROM\n");

        if(cdrom_io == 0)
        {
            cdrom_io = io;
            cdrom_drive = drive;
        }
    }
}


// ============================================================
// INITIALISE CD-ROM
// ============================================================

void cdrom_init(void)
{
    print("IDE DEVICE IDENTIFY\n");

    cdrom_io = 0;
    cdrom_drive = 0;

    // --------------------------------------------------------
    // Primary channel
    // --------------------------------------------------------

    check_device(
        PRIMARY_IO,
        0,
        "Primary Master"
    );

    check_device(
        PRIMARY_IO,
        1,
        "Primary Slave"
    );

    // --------------------------------------------------------
    // Secondary channel
    // --------------------------------------------------------

    check_device(
        SECONDARY_IO,
        0,
        "Secondary Master"
    );

    check_device(
        SECONDARY_IO,
        1,
        "Secondary Slave"
    );

    // --------------------------------------------------------
    // Restore Primary Master
    //
    // ATA disk driver uses Primary Master.
    // --------------------------------------------------------

    outb(
        PRIMARY_IO + ATA_DEVICE,
        0xA0
    );

    ide_delay();

    // --------------------------------------------------------
    // Final result
    // --------------------------------------------------------

    if(cdrom_io == 0)
    {
        print("No ATAPI CD-ROM found\n");
    }
    else
    {
        print("CD-ROM SELECTED\n");
    }

    print("IDENTIFY COMPLETE\n");
}


// ============================================================
// READ ONE CD-ROM SECTOR
// ============================================================

bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    if(cdrom_io == 0)
    {
        print("CD-ROM: No device\n");
        return false;
    }

    uint16_t io =
        cdrom_io;

    // --------------------------------------------------------
    // Select CD-ROM
    // --------------------------------------------------------

    outb(
        io + ATA_DEVICE,
        0xA0 | (cdrom_drive << 4)
    );

    ide_delay();

    // --------------------------------------------------------
    // Wait until ready
    // --------------------------------------------------------

    if(!wait_not_busy(io))
    {
        print("CD-ROM: Busy timeout\n");
        return false;
    }

    // --------------------------------------------------------
    // 2048-byte transfer
    // --------------------------------------------------------

    outb(
        io + ATA_FEATURES,
        0
    );

    outb(
        io + ATA_BYTE_COUNT_LOW,
        0x00
    );

    outb(
        io + ATA_BYTE_COUNT_HIGH,
        0x08
    );

    // --------------------------------------------------------
    // Send PACKET command
    // --------------------------------------------------------

    outb(
        io + ATA_COMMAND,
        ATA_PACKET
    );

    ide_delay();

    // --------------------------------------------------------
    // Wait for packet request
    // --------------------------------------------------------

    if(!wait_drq(io))
    {
        print("CD-ROM: PACKET timeout\n");
        dump_status(io);
        return false;
    }

    // --------------------------------------------------------
    // SCSI READ(10)
    // --------------------------------------------------------

    uint8_t packet[12];

    for(int i = 0; i < 12; i++)
    {
        packet[i] = 0;
    }

    packet[0] = 0x28;

    packet[2] =
        (uint8_t)(sector >> 24);

    packet[3] =
        (uint8_t)(sector >> 16);

    packet[4] =
        (uint8_t)(sector >> 8);

    packet[5] =
        (uint8_t)sector;

    packet[7] = 0;
    packet[8] = 1;

    // --------------------------------------------------------
    // Send packet
    // --------------------------------------------------------

    outsw(
        io + ATA_DATA,
        (uint16_t *)packet,
        6
    );

    // --------------------------------------------------------
    // Wait for data
    // --------------------------------------------------------

    if(!wait_drq(io))
    {
        print("CD-ROM: Data timeout\n");
        dump_status(io);
        return false;
    }

    // --------------------------------------------------------
    // Read 2048 bytes
    // --------------------------------------------------------

    insw(
        io + ATA_DATA,
        (uint16_t *)buffer,
        CD_SECTOR_SIZE / 2
    );

    return true;
}

