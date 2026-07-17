#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../include/types.h"

/*
    Custos Filesystem v1
*/

#define FS_MAGIC 0x43555354

#define FS_SECTOR_SIZE 512

/* Disk layout */

#define FS_SUPERBLOCK_SECTOR 1
#define FS_FILE_TABLE_START 2

/* Limits */

#define FS_MAX_FILES 128
#define FS_FILENAME_LENGTH 32
#define FS_MAX_FILE_SIZE 4096

/*
    File entry flags
*/

#define FILE_UNUSED 0
#define FILE_USED   1

/*
    Number of sectors needed for the file table.

    This is calculated automatically so it always
    matches the size of FileEntry.
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
    One file entry
*/

typedef struct
{
    char name[FS_FILENAME_LENGTH];

    uint32_t size;

    uint32_t start_sector;

    uint8_t used;

    /* keeps structure aligned */
    uint8_t reserved[3];

} FileEntry;

#define FS_FILE_TABLE_SECTORS \
((sizeof(FileEntry) * FS_MAX_FILES + FS_SECTOR_SIZE - 1) / FS_SECTOR_SIZE)

#define FS_DATA_START \
(FS_FILE_TABLE_START + FS_FILE_TABLE_SECTORS)

/*
    Filesystem
*/

typedef struct
{
    bool mounted;

    SuperBlock super;

    FileEntry files[FS_MAX_FILES];

} FileSystem;

/* Global filesystem */

extern FileSystem fs;

/* Core */

void fs_init(void);
void fs_format(void);
bool fs_mount(void);
void fs_sync(void);

/* File operations */

int fs_find(char *name);

bool fs_create(char *name);

bool fs_delete(char *name);

bool fs_write(char *name,
              char *data,
              uint32_t size);

bool fs_read(char *name,
             char *buffer);

uint32_t fs_size(char *name);

void fs_list(void);

#endif
