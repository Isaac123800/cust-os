#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../include/types.h"


/*
    Custos Filesystem v1

    Disk layout:

    Sector 0:
        Boot sector

    Sector 1:
        Superblock

    Sector 2-9:
        File table

    Sector 10+:
        File data blocks
*/


#define FS_MAGIC 0x43555354   // "CUST"

#define FS_SECTOR_SIZE 512


#define FS_SUPERBLOCK_SECTOR 1


#define FS_FILE_TABLE_START 2


#define FS_FILE_TABLE_SECTORS 8


#define FS_DATA_START 10



#define FS_MAX_FILES 128


#define FS_FILENAME_LENGTH 32


#define FS_MAX_FILE_SIZE 4096



/*
    File entry flags
*/

#define FILE_UNUSED 0

#define FILE_USED 1



/*
    Superblock

    Stores filesystem information
*/

typedef struct
{

    uint32_t magic;


    uint32_t total_sectors;


    uint32_t file_table_start;


    uint32_t data_start;


    uint32_t free_blocks;


} SuperBlock;



/*
    File table entry

    One entry represents one file
*/

typedef struct
{

    char name[FS_FILENAME_LENGTH];


    uint32_t size;


    uint32_t start_sector;


    uint8_t used;


} FileEntry;



/*
    Filesystem status
*/

typedef struct
{

    bool mounted;


    SuperBlock super;


    FileEntry files[FS_MAX_FILES];


} FileSystem;



/*
    Global filesystem instance
*/

extern FileSystem fs;



/*
    Initialize filesystem

    Loads filesystem from disk.
    Creates a filesystem if none exists.
*/

void fs_init(void);



/*
    Format disk

    Erases filesystem and creates
    a fresh one.
*/

void fs_format(void);



/*
    Mount filesystem
*/

bool fs_mount(void);



/*
    Save filesystem metadata
*/

void fs_sync(void);



/*
    Find a file

    Returns file index,
    or -1 if not found.
*/

int fs_find(char *name);



/*
    Create a new file
*/

bool fs_create(char *name);



/*
    Delete a file
*/

bool fs_delete(char *name);



/*
    Write data to file
*/

bool fs_write(char *name, char *data, uint32_t size);



/*
    Read file contents
*/

bool fs_read(char *name, char *buffer);



/*
    Get file size
*/

uint32_t fs_size(char *name);



/*
    List all files

    Used by ls command
*/

void fs_list(void);



#endif
