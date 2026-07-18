#ifndef CDROM_H
#define CDROM_H

#include "../include/types.h"

#define CD_SECTOR_SIZE 2048

void cdrom_init(void);

bool cdrom_read_sector(
    uint32_t sector,
    uint8_t *buffer
);

#endif
