#ifndef DISK_H
#define DISK_H

#include "../include/types.h"


/*
    Disk constants
*/

#define DISK_SECTOR_SIZE 512



/*
    Initialize disk subsystem
*/

void disk_init(void);



/*
    Read one sector

    sector:
        LBA sector number

    buffer:
        destination memory
*/

bool disk_read(
    uint32_t sector,
    uint8_t *buffer
);



/*
    Write one sector

    sector:
        LBA sector number

    buffer:
        source memory
*/

bool disk_write(
    uint32_t sector,
    const uint8_t *buffer
);



/*
    Flush disk cache
*/

void disk_flush(void);



#endif
