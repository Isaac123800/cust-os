#include "filesystem.h"

#include "../drivers/ata.h"


/*
    Global filesystem
*/

FileSystem fs;



/*
    Internal helper
    Calculate how many sectors
    a file needs.
*/

static uint32_t sectors_needed(uint32_t size)
{
    uint32_t sectors;

    sectors = size / FS_SECTOR_SIZE;

    if(size % FS_SECTOR_SIZE)
        sectors++;

    return sectors;
}



/*
    Calculate the first data sector
    for a file index.
*/

static uint32_t file_sector(uint32_t index)
{
    return FS_DATA_START + (index * 8);
}



/*
    Clear memory buffer
*/

static void memset(void *ptr, uint8_t value, uint32_t size)
{
    uint8_t *p = ptr;

    for(uint32_t i = 0; i < size; i++)
        p[i] = value;
}



/*
    Copy string
*/

static void strcpy(char *dest, char *src)
{
    int i = 0;

    while(src[i])
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = 0;
}



/*
    Compare strings
*/

static bool strcmp(char *a, char *b)
{
    int i = 0;


    while(a[i] && b[i])
    {
        if(a[i] != b[i])
            return false;

        i++;
    }


    return a[i] == b[i];
}



/*
    Save the file table
    to disk
*/

static void save_file_table()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    uint8_t *table =
        (uint8_t*)fs.files;


    uint32_t size =
        sizeof(FileEntry) * FS_MAX_FILES;


    uint32_t offset = 0;


    uint32_t sector =
        FS_FILE_TABLE_START;



    while(offset < size)
    {
        memset(buffer,0,FS_SECTOR_SIZE);


        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++,offset++)
        {
            buffer[i] = table[offset];
        }


        ata_write_sector(sector,buffer);

        sector++;
    }
}



/*
    Load file table
*/

static void load_file_table()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    uint8_t *table =
        (uint8_t*)fs.files;


    uint32_t size =
        sizeof(FileEntry) * FS_MAX_FILES;


    uint32_t offset = 0;


    uint32_t sector =
        FS_FILE_TABLE_START;



    while(offset < size)
    {

        ata_read_sector(sector,buffer);



        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++,offset++)
        {
            table[offset] = buffer[i];
        }


        sector++;
    }
}



/*
    Save superblock
*/

static void save_superblock()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    memset(buffer,0,FS_SECTOR_SIZE);



    uint8_t *data =
        (uint8_t*)&fs.super;



    for(uint32_t i = 0;
        i < sizeof(SuperBlock);
        i++)
    {
        buffer[i] = data[i];
    }



    ata_write_sector(
        FS_SUPERBLOCK_SECTOR,
        buffer
    );
}



/*
    Load superblock
*/

static bool load_superblock()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    ata_read_sector(
        FS_SUPERBLOCK_SECTOR,
        buffer
    );



    uint8_t *data =
        (uint8_t*)&fs.super;



    for(uint32_t i = 0;
        i < sizeof(SuperBlock);
        i++)
    {
        data[i] = buffer[i];
    }



    if(fs.super.magic != FS_MAGIC)
        return false;


    return true;
}
