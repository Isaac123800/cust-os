#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../include/types.h"


#define FS_MAGIC 0x43555354   /* "CUST" */

#define FS_SECTOR_SIZE 512


#define FS_SUPERBLOCK_SECTOR 1

#define FS_FILE_TABLE_START 2

#define FS_DATA_START 10



#define FS_MAX_FILES 64

#define FS_FILENAME_LENGTH 32

#define FS_MAX_FILE_SIZE 65536



#define FILE_UNUSED 0
#define FILE_USED   1



typedef struct
{
    uint32_t magic;

    uint32_t total_sectors;

    uint32_t file_table_start;

    uint32_t data_start;

    uint32_t free_blocks;

} SuperBlock;




typedef struct
{
    char name[FS_FILENAME_LENGTH];

    uint32_t size;

    uint32_t start_sector;

    uint8_t used;

} FileEntry;




typedef struct
{
    SuperBlock super;

    FileEntry files[FS_MAX_FILES];

    bool mounted;

} FileSystem;



/*
    Filesystem functions
*/


void fs_init(void);


bool fs_mount(void);


/*
    Changed from void to bool

    Returns false if disk writes fail
*/

bool fs_format(void);



void fs_sync(void);



bool fs_create(
    char *name
);



bool fs_write(
    char *name,
    char *data,
    uint32_t size
);



bool fs_read(
    char *name,
    char *buffer
);



bool fs_delete(
    char *name
);



int fs_find(
    char *name
);



uint32_t fs_size(
    char *name
);



void fs_list(void);



#endif
