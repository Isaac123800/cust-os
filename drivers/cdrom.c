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

    while(timeout--)
    {
        uint8_t status = inb(io + ATA_STATUS);

        if(!(status & ATA_BSY))
            return true;
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
        uint8_t status = inb(io + ATA_STATUS);


        if(status & ATA_ERR)
            return false;


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
// ============================================================

static uint8_t ide_identify(
    uint16_t io,
    uint8_t drive
)
{
    outb(
        io + ATA_DEVICE,
        0xA0 | (drive << 4)
    );


    ide_delay();


    outb(io + ATA_SECTOR_COUNT, 0);
    outb(io + ATA_LBA_LOW, 0);
    outb(io + ATA_LBA_MID, 0);
    outb(io + ATA_LBA_HIGH, 0);


    outb(
        io + ATA_COMMAND,
        ATA_IDENTIFY
    );


    uint8_t status =
        inb(io + ATA_STATUS);



    if(status == 0)
        return 0;



    if(!wait_not_busy(io))
        return 0;



    uint8_t mid =
        inb(io + ATA_LBA_MID);

    uint8_t high =
        inb(io + ATA_LBA_HIGH);



    // ATAPI signature

    if(mid == 0x14 &&
       high == 0xEB)
    {
        return 2;
    }



    status = inb(io + ATA_STATUS);


    if(status & ATA_ERR)
        return 0;



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
        ide_identify(io, drive);



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
// READ ONE ISO9660 SECTOR
// ============================================================

bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{
    if(cdrom_io == 0)
    {
        print("ATAPI: No CD-ROM\n");
        return false;
    }


    uint16_t io = cdrom_io;


    uint8_t packet[12];


    for(int i = 0; i < 12; i++)
        packet[i] = 0;



    // --------------------------------------------------------
    // SCSI READ(10)
    // --------------------------------------------------------

    packet[0] = 0x28;


    // LBA (big endian)

    packet[2] = (sector >> 24) & 0xFF;
    packet[3] = (sector >> 16) & 0xFF;
    packet[4] = (sector >> 8) & 0xFF;
    packet[5] = sector & 0xFF;


    // Transfer length = 1 sector

    packet[7] = 0;
    packet[8] = 1;



    print("ATAPI: READ SECTOR\n");



    // --------------------------------------------------------
    // Select drive
    // --------------------------------------------------------

    outb(
        io + ATA_DEVICE,
        0xA0 | (cdrom_drive << 4)
    );


    ide_delay();



    if(!wait_not_busy(io))
    {
        print("ATAPI: Drive busy\n");
        dump_status(io);
        return false;
    }



    // --------------------------------------------------------
    // Set transfer size
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


    print("ATAPI: PACKET SENT\n");



    if(!wait_drq(io))
    {
        print("ATAPI: No command DRQ\n");
        dump_status(io);
        return false;
    }



    uint8_t reason =
        inb(io + ATA_INTERRUPT_REASON);



    if(!(reason & ATAPI_COD) ||
        (reason & ATAPI_IO))
    {
        print("ATAPI: Bad command phase\n");
        dump_status(io);
        return false;
    }



    // --------------------------------------------------------
    // Send READ(10) packet
    // --------------------------------------------------------

    outsw(
        io + ATA_DATA,
        packet,
        6
    );


    print("ATAPI: READ10 SENT\n");



    // --------------------------------------------------------
    // Wait for data phase
    // --------------------------------------------------------

    int timeout = 5000000;


    while(timeout--)
    {
        uint8_t status =
            inb(io + ATA_STATUS);


        if(status & ATA_ERR)
        {
            print("ATAPI: ERROR\n");
            dump_status(io);
            return false;
        }


        if(status & ATA_DRQ)
            break;
    }



    if(timeout <= 0)
    {
        print("ATAPI: DATA TIMEOUT\n");
        dump_status(io);
        return false;
    }



    reason =
        inb(io + ATA_INTERRUPT_REASON);



    print("ATAPI REASON=");
    print_hex(reason);
    print("\n");



    if((reason & ATAPI_COD) ||
       !(reason & ATAPI_IO))
    {
        print("ATAPI: Wrong data phase\n");
        return false;
    }



    uint16_t bytes =
        inb(io + ATA_BYTE_COUNT_LOW) |
        (inb(io + ATA_BYTE_COUNT_HIGH) << 8);



    print("ATAPI BYTES=");
    print_hex(bytes >> 8);
    print_hex(bytes & 0xFF);
    print("\n");



    if(bytes == 0 || bytes > 2048)
    {
        print("ATAPI: Invalid size\n");
        return false;
    }



    // --------------------------------------------------------
    // Read sector data
    // --------------------------------------------------------

    insw(
        io + ATA_DATA,
        buffer,
        bytes / 2
    );


    print("ATAPI: DATA READ\n");



    // Clear final status

    uint8_t status =
        inb(io + ATA_STATUS);



    if(status & ATA_ERR)
    {
        print("ATAPI: FINAL ERROR\n");
        dump_status(io);
        return false;
    }



    print("ATAPI: READ SUCCESS\n");


    return true;
}
