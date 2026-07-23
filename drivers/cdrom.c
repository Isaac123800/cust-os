#include "cdrom.h"

#include "../include/types.h"
#include "io.h"

extern void print(char *text);
extern void print_hex(uint8_t value);


// ------------------------------------------------------------
// IDE Ports
// ------------------------------------------------------------

#define PRIMARY_IO      0x1F0
#define SECONDARY_IO    0x170


// ------------------------------------------------------------
// ATA Registers
// ------------------------------------------------------------

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



// ------------------------------------------------------------
// ATA Commands
// ------------------------------------------------------------

#define ATA_IDENTIFY            0xEC
#define ATA_PACKET              0xA0



// ------------------------------------------------------------
// ATAPI Commands
// ------------------------------------------------------------

#define ATAPI_READ10            0x28



// ------------------------------------------------------------
// Status bits
// ------------------------------------------------------------

#define ATA_ERR     0x01
#define ATA_DRQ     0x08
#define ATA_DF      0x20
#define ATA_DRDY    0x40
#define ATA_BSY     0x80



// ------------------------------------------------------------
// ATAPI Interrupt Reason bits
// ------------------------------------------------------------

#define ATAPI_COD      0x01
#define ATAPI_IO       0x02



// ------------------------------------------------------------
// Selected CD-ROM
// ------------------------------------------------------------

static uint16_t cdrom_io = 0;
static uint8_t  cdrom_drive = 0;



// ------------------------------------------------------------
// Delay
// ------------------------------------------------------------

static void ide_delay(void)
{
    inb(0x80);
    inb(0x80);
    inb(0x80);
    inb(0x80);
}



// ------------------------------------------------------------
// Wait for BSY to clear
// ------------------------------------------------------------

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



// ------------------------------------------------------------
// Wait for DRQ
// ------------------------------------------------------------

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



// ------------------------------------------------------------
// Debug status
// ------------------------------------------------------------

static void dump_status(uint16_t io)
{
    uint8_t status = inb(io + ATA_STATUS);
    uint8_t error  = inb(io + ATA_ERROR);
    uint8_t reason = inb(io + ATA_INTERRUPT_REASON);


    print("STATUS=");
    print_hex(status);

    print(" ERROR=");
    print_hex(error);

    print(" REASON=");
    print_hex(reason);

    print("\n");
}
// ------------------------------------------------------------
// Identify IDE device
// ------------------------------------------------------------

static uint8_t ide_identify(uint16_t io, uint8_t drive)
{
    outb(io + ATA_DEVICE, 0xA0 | (drive << 4));

    ide_delay();


    outb(io + ATA_SECTOR_COUNT, 0);
    outb(io + ATA_LBA_LOW, 0);
    outb(io + ATA_LBA_MID, 0);
    outb(io + ATA_LBA_HIGH, 0);


    outb(io + ATA_COMMAND, ATA_IDENTIFY);


    uint8_t status = inb(io + ATA_STATUS);


    // No device
    if(status == 0)
        return 0;


    if(!wait_not_busy(io))
        return 0;



    uint8_t mid  = inb(io + ATA_LBA_MID);
    uint8_t high = inb(io + ATA_LBA_HIGH);



    // ATAPI signature
    if(mid == 0x14 && high == 0xEB)
        return 2;



    status = inb(io + ATA_STATUS);


    if(status & ATA_ERR)
        return 0;



    return 1;
}




// ------------------------------------------------------------
// Check one device
// ------------------------------------------------------------

static void check_device(
    uint16_t io,
    uint8_t drive,
    char *name
)
{
    uint8_t result = ide_identify(io, drive);



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


        // Save first CD-ROM found
        if(cdrom_io == 0)
        {
            cdrom_io = io;
            cdrom_drive = drive;
        }
    }
}




// ------------------------------------------------------------
// Initialize CD-ROM
// ------------------------------------------------------------

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

        if(cdrom_io == PRIMARY_IO)
            print("PRIMARY CHANNEL\n");
        else
            print("SECONDARY CHANNEL\n");


        if(cdrom_drive == 0)
            print("MASTER DRIVE\n");
        else
            print("SLAVE DRIVE\n");
    }



    print("IDENTIFY COMPLETE\n");
}
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


    print("ATAPI: READ START\n");



    // Select detected CD-ROM
    outb(
        io + ATA_DEVICE,
        0xA0 | (cdrom_drive << 4)
    );


    ide_delay();



    // Wait until drive is ready
    if(!wait_not_busy(io))
    {
        print("ATAPI: Drive busy timeout\n");
        dump_status(io);
        return false;
    }


    print("ATAPI: DRIVE READY\n");



    /*
        Set maximum transfer size.

        The drive will not transfer more than
        this amount during one phase.
    */

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



    // Send ATAPI PACKET command

    outb(
        io + ATA_COMMAND,
        ATA_PACKET
    );



    print("ATAPI: PACKET COMMAND SENT\n");



    /*
        After PACKET the drive should enter
        the command phase:

        BSY -> DRQ

        The host then sends the 12 byte packet.
    */

    if(!wait_drq(io))
    {
        print("ATAPI: No command DRQ\n");
        dump_status(io);
        return false;
    }



    uint8_t reason =
        inb(io + ATA_INTERRUPT_REASON);



    print("ATAPI: COMMAND PHASE REASON=");
    print_hex(reason);
    print("\n");



    /*
        We expect:

        COD = 1
        IO  = 0

        Meaning:
        "Device wants a command from host"
    */

    if(
        !(reason & ATAPI_COD) ||
        (reason & ATAPI_IO)
      )
    {
        print("ATAPI: Invalid command phase\n");
        return false;
    }
    // --------------------------------------------------------
    // Build ATAPI READ(10) packet
    // --------------------------------------------------------

    for(int i = 0; i < 12; i++)
        packet[i] = 0;



    /*
        SCSI READ(10)

        Byte 0:
            Operation code

        Bytes 2-5:
            LBA

        Bytes 7-8:
            Transfer length
    */

    packet[0] = ATAPI_READ10;



    // LBA (big endian)

    packet[2] = (sector >> 24) & 0xFF;
    packet[3] = (sector >> 16) & 0xFF;
    packet[4] = (sector >> 8)  & 0xFF;
    packet[5] = sector & 0xFF;



    // Read exactly one sector

    packet[7] = 0;
    packet[8] = 1;



    print("ATAPI: SENDING READ10\n");



    /*
        Send 12-byte ATAPI command packet.

        ATA data register is 16-bit,
        therefore 12 bytes = 6 words.
    */

    outsw(
        io + ATA_DATA,
        packet,
        6
    );



    print("ATAPI: READ10 SENT\n");



    // Wait for the device response

    if(!wait_not_busy(io))
    {
        print("ATAPI: Busy after packet\n");
        dump_status(io);
        return false;
    }



    if(!wait_drq(io))
{
    print("ATAPI: No DATA DRQ\n");

    uint8_t status = inb(io + ATA_STATUS);
    uint8_t error = inb(io + ATA_ERROR);
    uint8_t reason = inb(io + ATA_INTERRUPT_REASON);
    uint8_t count_low = inb(io + ATA_BYTE_COUNT_LOW);
    uint8_t count_high = inb(io + ATA_BYTE_COUNT_HIGH);

    print("STATUS=");
    print_hex(status);

    print(" ERROR=");
    print_hex(error);

    print(" REASON=");
    print_hex(reason);

    print(" COUNT=");
    print_hex(count_high);
    print_hex(count_low);

    print("\n");

    return false;
}



   reason = inb(io + ATA_INTERRUPT_REASON);


print("ATAPI: DATA PHASE REASON=");
print_hex(reason);
print("\n");







    /*
        Data phase should be:

        COD = 0
        IO  = 1

        Meaning:
        Device has data for host.
    */

    if(
        (reason & ATAPI_COD) ||
        !(reason & ATAPI_IO)
      )
    {
        print("ATAPI: Invalid data phase\n");
        return false;
    }



    /*
        The drive tells us the amount
        of data it will send.
    */

    uint16_t bytes =
        inb(io + ATA_BYTE_COUNT_LOW) |
        (inb(io + ATA_BYTE_COUNT_HIGH) << 8);



    print("ATAPI: BYTE COUNT=");
    print_hex(bytes >> 8);
    print_hex(bytes & 0xFF);
    print("\n");



    if(bytes == 0 || bytes > 2048)
    {
        print("ATAPI: Bad transfer size\n");
        return false;
    }
    // --------------------------------------------------------
    // Read sector data
    // --------------------------------------------------------

    print("ATAPI: READING DATA\n");


    insw(
        io + ATA_DATA,
        buffer,
        bytes / 2
    );


    print("ATAPI: DATA TRANSFER COMPLETE\n");



    /*
        Acknowledge the completed transfer.

        Some ATAPI devices/emulators expect
        the status register to be read after
        the data phase.
    */

    uint8_t status =
        inb(io + ATA_STATUS);



    // Wait for final completion

    if(!wait_not_busy(io))
    {
        print("ATAPI: Final busy timeout\n");
        dump_status(io);
        return false;
    }



    status = inb(io + ATA_STATUS);



    if(status & ATA_ERR)
    {
        print("ATAPI: Final command error\n");
        dump_status(io);
        return false;
    }



    if(status & ATA_DF)
    {
        print("ATAPI: Device fault\n");
        dump_status(io);
        return false;
    }



    print("ATAPI: READ SUCCESS\n");


    return true;
}
