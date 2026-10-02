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
// COMMANDS
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
// ATAPI REASON BITS
// ============================================================

#define ATAPI_COD  0x01
#define ATAPI_IO   0x02


// ============================================================
// CD-ROM STATE
// ============================================================

static uint16_t cdrom_io = 0;
static uint8_t cdrom_drive = 0;


// ============================================================
// DELAY
// ============================================================

static void ide_delay(void)
{
    inb(0x80);
    inb(0x80);
    inb(0x80);
    inb(0x80);
}


// ============================================================
// WAIT UNTIL NOT BUSY
// ============================================================

static bool wait_not_busy(uint16_t io)
{
    int timeout = 5000000;

    while (timeout--)
    {
        uint8_t status = inb(io + ATA_STATUS);

        if (!(status & ATA_BSY))
            return true;
    }

    return false;
}


// ============================================================
// WAIT FOR DRQ
// ============================================================

static bool wait_drq(uint16_t io)
{
    int timeout = 5000000;

    while (timeout--)
    {
        uint8_t status = inb(io + ATA_STATUS);

        if (status & ATA_ERR)
            return false;

        if (!(status & ATA_BSY) && (status & ATA_DRQ))
            return true;
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
// Return values:
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
    // Check status before command
    // --------------------------------------------------------

    uint8_t status =
        inb(io + ATA_STATUS);


    if (status == 0xFF)
        return 0;


    // --------------------------------------------------------
    // Try ATA IDENTIFY
    // --------------------------------------------------------

    outb(
        io + ATA_COMMAND,
        ATA_IDENTIFY
    );


    ide_delay();


    status =
        inb(io + ATA_STATUS);


    // --------------------------------------------------------
    // No device
    // --------------------------------------------------------

    if (status == 0)
        return 0;


    // --------------------------------------------------------
    // Wait until device isn't busy
    // --------------------------------------------------------

    if (!wait_not_busy(io))
        return 0;


    // --------------------------------------------------------
    // Check ATAPI signature
    //
    // ATAPI normally reports:
    //
    // LBA MID  = 0x14
    // LBA HIGH = 0xEB
    // --------------------------------------------------------

    uint8_t mid =
        inb(io + ATA_LBA_MID);

    uint8_t high =
        inb(io + ATA_LBA_HIGH);


    if (mid == 0x14 &&
        high == 0xEB)
    {
        return 2;
    }


    // --------------------------------------------------------
    // If IDENTIFY succeeded, it is an ATA device
    // --------------------------------------------------------

    status =
        inb(io + ATA_STATUS);


    if (status & ATA_ERR)
    {
        /*
         * IDENTIFY failed.
         *
         * It may still be an ATAPI device, so explicitly
         * try IDENTIFY PACKET DEVICE.
         */
    }
    else
    {
        return 1;
    }


    // --------------------------------------------------------
    // ATAPI IDENTIFY PACKET DEVICE
    // --------------------------------------------------------

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


    status =
        inb(io + ATA_STATUS);


    if (status == 0)
        return 0;


    if (!wait_not_busy(io))
        return 0;


    status =
        inb(io + ATA_STATUS);


    if (status & ATA_ERR)
        return 0;


    // --------------------------------------------------------
    // IDENTIFY PACKET should have data ready
    // --------------------------------------------------------

    if (!(status & ATA_DRQ))
        return 0;


    // --------------------------------------------------------
    // Consume the 512-byte identify packet.
    //
    // We don't need the identification strings yet, but the
    // device's data phase must be drained.
    // --------------------------------------------------------

    uint16_t identify_buffer[256];


    insw(
        io + ATA_DATA,
        identify_buffer,
        256
    );


    return 2;
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


    if (result == 0)
    {
        print(": Empty\n");
    }
    else if (result == 1)
    {
        print(": ATA Disk\n");
    }
    else if (result == 2)
    {
        print(": ATAPI CD-ROM\n");


        if (cdrom_io == 0)
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
    // Result
    // --------------------------------------------------------

    if (cdrom_io == 0)
    {
        print("No ATAPI CD-ROM found\n");
    }
    else
    {
        print("CD-ROM SELECTED\n");
    }


    print("IDENTIFY COMPLETE\n");
}
