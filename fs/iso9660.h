#ifndef ISO9660_H
#define ISO9660_H

#include "../include/types.h"


#define ISO_SECTOR_SIZE 2048


void iso_init(void);


bool iso_read_file(
    char *name,
    uint8_t *buffer,
    uint32_t *size
);


void iso_list_root(void);


#endif
