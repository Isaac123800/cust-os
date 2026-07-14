#include "disk.h"

#include "ata.h"



/*
    Initialize disk system
*/

void disk_init(void)
{
    ata_init();
}





/*
    Read one sector

    Passes request to ATA driver
*/

bool disk_read(
    uint32_t sector,
    uint8_t *buffer
)
{
    return ata_read_sector(
        sector,
        buffer
    );
}





/*
    Write one sector

    Passes request to ATA driver
*/

bool disk_write(
    uint32_t sector,
    const uint8_t *buffer
)
{
    return ata_write_sector(
        sector,
        buffer
    );
}





/*
    Flush disk cache
*/

void disk_flush(void)
{
    ata_flush();
}
