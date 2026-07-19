//
// CustOS ATAPI CD-ROM Driver
// Improved Version
// Part 1/5
//

#include "cdrom.h"
#include "io.h"

#include <stdbool.h>

#include "../include/types.h"


extern void print(char *text);



#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_FEATURES    1
#define ATA_SECCOUNT0   2

#define ATA_LBA0        3
#define ATA_LBA1        4
#define ATA_LBA2        5

#define ATA_HDDEVSEL    6
#define ATA_COMMAND     7
#define ATA_STATUS      7



#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1



#define ATAPI_READ10            0x28



#define ATA_MASTER 0xA0
#define ATA_SLAVE  0xB0



#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08
#define STATUS_ERR 0x01




typedef struct
{

    uint16_t base;

    uint8_t drive;

    bool found;

} CDDevice;




static CDDevice cdrom;



/*
    ATA PORT HELPERS
*/


static void out_port(
    uint16_t base,
    uint8_t reg,
    uint8_t value
)
{

    outb(
        base + reg,
        value
    );

}




static uint8_t in_port(
    uint16_t base,
    uint8_t reg
)
{

    return inb(
        base + reg
    );

}





/*
    ATA 400ns DELAY
*/


static void ata_delay(
    uint16_t base
)
{

    in_port(base, ATA_STATUS);
    in_port(base, ATA_STATUS);
    in_port(base, ATA_STATUS);
    in_port(base, ATA_STATUS);

}

// 
// WAIT UNTIL DEVICE IS NOT BUSY
//


static bool wait_not_busy(
    uint16_t base
)
{

    int timeout = 1000000;


    while(timeout--)
    {

        uint8_t status =
        in_port(
            base,
            ATA_STATUS
        );



        if(status & STATUS_ERR)
            return false;



        if(!(status & STATUS_BSY))
            return true;

    }


    return false;

}





//
// WAIT FOR DATA REQUEST
//


static bool wait_drq(
    uint16_t base
)
{

    int timeout = 1000000;


    while(timeout--)
    {

        uint8_t status =
        in_port(
            base,
            ATA_STATUS
        );



        if(status & STATUS_ERR)
            return false;



        if(status & STATUS_DRQ)
            return true;

    }


    return false;

}






//
// IDENTIFY ATAPI DEVICE
//


static bool detect_device(
    uint16_t base,
    uint8_t drive
)
{


    out_port(
        base,
        ATA_HDDEVSEL,
        drive
    );


    ata_delay(base);



    /*
        Reset registers
    */


    out_port(
        base,
        ATA_SECCOUNT0,
        0
    );


    out_port(
        base,
        ATA_LBA0,
        0
    );


    out_port(
        base,
        ATA_LBA1,
        0
    );


    out_port(
        base,
        ATA_LBA2,
        0
    );




    out_port(
        base,
        ATA_COMMAND,
        ATA_CMD_IDENTIFY_PACKET
    );



    uint8_t status =
    in_port(
        base,
        ATA_STATUS
    );



    if(status == 0)
        return false;



    if(!wait_not_busy(base))
        return false;



    /*
        Check ATAPI signature
    */


    uint8_t sig1 =
    in_port(
        base,
        ATA_LBA1
    );


    uint8_t sig2 =
    in_port(
        base,
        ATA_LBA2
    );



    if(
        !(
            (sig1 == 0x14 && sig2 == 0xEB)
            ||
            (sig1 == 0x69 && sig2 == 0x96)
        )
    )
    {
        return false;
    }



    if(!wait_drq(base))
        return false;



    /*
        Read IDENTIFY PACKET data
        256 words = 512 bytes
    */


    uint16_t identify[256];


    insw(
        base + ATA_DATA,
        identify,
        256
    );



    cdrom.base =
    base;


    cdrom.drive =
    drive;


    cdrom.found =
    true;



    return true;

}

// 
// INITIALIZE CD-ROM DRIVE
//


void cdrom_init(void)
{

    print("Searching ATAPI CD-ROM...\n");


    cdrom.found = false;



    /*
        ATA channel base addresses
    */


    uint16_t channels[] =
    {
        0x1F0,   // Primary
        0x170    // Secondary
    };



    for(int i = 0; i < 2; i++)
    {

        /*
            Check master device
        */


        if(detect_device(
            channels[i],
            ATA_MASTER
        ))
        {

            print("ATAPI CD-ROM found\n");

            return;

        }



        /*
            Check slave device
        */


        if(detect_device(
            channels[i],
            ATA_SLAVE
        ))
        {

            print("ATAPI CD-ROM found\n");

            return;

        }

    }



    print("No ATAPI CD-ROM found\n");

}

// 
// READ ONE CD-ROM SECTOR
//


bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
)
{

    if(!cdrom.found)
    {
        return false;
    }



    uint16_t base =
    cdrom.base;



    /*
        Select CD-ROM drive
    */


    out_port(
        base,
        ATA_HDDEVSEL,
        cdrom.drive
    );


    ata_delay(base);



    if(!wait_not_busy(base))
    {
        return false;
    }




    /*
        Set transfer size
        2048 bytes
    */


    out_port(
        base,
        ATA_FEATURES,
        0
    );


    out_port(
        base,
        ATA_SECCOUNT0,
        0
    );


    out_port(
        base,
        ATA_LBA1,
        0x00
    );


    out_port(
        base,
        ATA_LBA2,
        0x08
    );




    /*
        Send PACKET command
    */


    out_port(
        base,
        ATA_COMMAND,
        ATA_CMD_PACKET
    );



    if(!wait_drq(base))
    {
        print("ATAPI PACKET failed\n");

        return false;
    }




    /*
        ATAPI READ(10) command
    */


    uint8_t packet[12];



    for(int i = 0; i < 12; i++)
    {
        packet[i] = 0;
    }



    packet[0] = ATAPI_READ10;



    /*
        Logical Block Address
    */


    packet[2] =
    (sector >> 24) & 0xFF;


    packet[3] =
    (sector >> 16) & 0xFF;


    packet[4] =
    (sector >> 8) & 0xFF;


    packet[5] =
    sector & 0xFF;



    /*
        Transfer one sector
    */


    packet[8] = 0;

    packet[9] = 1;



    /*
        Send 12 byte packet
        = 6 words
    */


    outsw(
        base + ATA_DATA,
        packet,
        6
    );



    if(!wait_drq(base))
    {
        print("ATAPI READ failed\n");

        return false;
    }




    /*
        Read 2048 bytes
        1024 words
    */


    insw(
        base + ATA_DATA,
        buffer,
        1024
    );



    return true;

}

// 
// CHECK IF CD-ROM EXISTS
//


bool cdrom_available()
{

    return cdrom.found;

}





//
// GET CURRENT CD-ROM DEVICE INFO
//


uint16_t cdrom_get_base()
{

    return cdrom.base;

}





uint8_t cdrom_get_drive()
{

    return cdrom.drive;

}





//
// TEST READ FUNCTION
//


bool cdrom_test()
{

    if(!cdrom.found)
    {
        print("No CD-ROM device\n");

        return false;
    }



    uint8_t buffer[2048];



    if(cdrom_read_sector(
        16,
        buffer
    ))
    {

        print("CD-ROM read OK\n");

        return true;

    }



    print("CD-ROM read failed\n");


    return false;

}
