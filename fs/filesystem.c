#include "filesystem.h"

#include "../drivers/disk.h"



/*
    Global filesystem
*/

FileSystem fs;



/*
    Internal memory helpers
*/

static void memset(void *ptr, uint8_t value, uint32_t size)
{
    uint8_t *p = ptr;

    for(uint32_t i = 0; i < size; i++)
    {
        p[i] = value;
    }
}



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



static bool string_equal(char *a, char *b)
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



static uint32_t sectors_needed(uint32_t size)
{
    uint32_t sectors;

    sectors = size / FS_SECTOR_SIZE;


    if(size % FS_SECTOR_SIZE)
        sectors++;


    return sectors;
}





/*
    Save superblock
*/

static void save_superblock()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    memset(
        buffer,
        0,
        FS_SECTOR_SIZE
    );


    uint8_t *data =
        (uint8_t *)&fs.super;



    for(uint32_t i = 0;
        i < sizeof(SuperBlock);
        i++)
    {
        buffer[i] = data[i];
    }



    disk_write(
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


    if(!disk_read(
        FS_SUPERBLOCK_SECTOR,
        buffer))
    {
        return false;
    }



    uint8_t *data =
        (uint8_t *)&fs.super;



    for(uint32_t i = 0;
        i < sizeof(SuperBlock);
        i++)
    {
        data[i] = buffer[i];
    }



    if(fs.super.magic != FS_MAGIC)
    {
        return false;
    }
  


    return true;
}
/*
    Save file table to disk
*/

static void save_file_table()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    uint8_t *table =
        (uint8_t *)fs.files;


    uint32_t size =
        sizeof(FileEntry) * FS_MAX_FILES;


    uint32_t offset = 0;


    uint32_t sector =
        FS_FILE_TABLE_START;



    while(offset < size)
    {
        memset(
            buffer,
            0,
            FS_SECTOR_SIZE
        );



        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++, offset++)
        {
            buffer[i] = table[offset];
        }



        disk_write(
            sector,
            buffer
        );


        sector++;
    }
}





/*
    Load file table from disk
*/

static void load_file_table()
{
    uint8_t buffer[FS_SECTOR_SIZE];


    uint8_t *table =
        (uint8_t *)fs.files;



    uint32_t size =
        sizeof(FileEntry) * FS_MAX_FILES;



    uint32_t offset = 0;



    uint32_t sector =
        FS_FILE_TABLE_START;



    while(offset < size)
    {
        disk_read(
            sector,
            buffer
        );



        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++, offset++)
        {
            table[offset] = buffer[i];
        }



        sector++;
    }
}





/*
    Format filesystem

    Creates a fresh filesystem
*/

void fs_format(void)
{
    memset(
        &fs,
        0,
        sizeof(FileSystem)
    );



    fs.super.magic =
        FS_MAGIC;



    fs.super.total_sectors =
        1024 * 1024;



    fs.super.file_table_start =
        FS_FILE_TABLE_START;



    fs.super.data_start =
        FS_DATA_START;



    fs.super.free_blocks =
        10000;



    fs.mounted = true;



    save_superblock();


    save_file_table();
}





/*
    Mount filesystem
*/

bool fs_mount(void)
{

    if(!load_superblock())
    {
        return false;
    }



    load_file_table();



    fs.mounted = true;


    return true;
}





/*
    Initialize filesystem
*/

void fs_init(void)
{

    if(fs_mount())
    {
        return;
    }



    fs_format();
}





/*
    Save filesystem changes
*/

void fs_sync(void)
{
    if(!fs.mounted)
        return;



    save_superblock();


    save_file_table();
}
/*
    Find file by name

    Returns:
        file index
        -1 if not found
*/

int fs_find(char *name)
{
    for(int i = 0;
        i < FS_MAX_FILES;
        i++)
    {
        if(fs.files[i].used == FILE_USED)
        {
            if(string_equal(
                fs.files[i].name,
                name))
            {
                return i;
            }
        }
    }


    return -1;
}





/*
    Find unused file entry
*/

static int find_free_entry()
{
    for(int i = 0;
        i < FS_MAX_FILES;
        i++)
    {
        if(fs.files[i].used == FILE_UNUSED)
        {
            return i;
        }
    }


    return -1;
}





/*
    Basic sector allocator

    Temporary version.

    Later this will be replaced
    with a filesystem bitmap.
*/

static uint32_t next_free_sector =
    FS_DATA_START;



static uint32_t allocate_sector()
{
    uint32_t sector =
        next_free_sector;


    next_free_sector++;


    return sector;
}





/*
    Create file

    Equivalent to shell:
    touch filename
*/

bool fs_create(char *name)
{

    if(fs_find(name) >= 0)
    {
        return false;
    }



    int index =
        find_free_entry();



    if(index < 0)
    {
        return false;
    }



    FileEntry *file =
        &fs.files[index];



    memset(
        file,
        0,
        sizeof(FileEntry)
    );



    strcpy(
        file->name,
        name
    );



    file->used =
        FILE_USED;



    file->size =
        0;



    file->start_sector =
        allocate_sector();



    fs_sync();



    return true;
}
/*
    Write data to file

    Saves file contents
    to disk sectors.
*/

bool fs_write(
    char *name,
    char *data,
    uint32_t size
)
{
    int index =
        fs_find(name);



    if(index < 0)
    {
        return false;
    }



    if(size > FS_MAX_FILE_SIZE)
    {
        return false;
    }



    FileEntry *file =
        &fs.files[index];



    uint32_t sectors =
        sectors_needed(size);



    uint32_t sector =
        file->start_sector;



    uint32_t offset = 0;



    uint8_t buffer[FS_SECTOR_SIZE];



    for(uint32_t s = 0;
        s < sectors;
        s++)
    {

        memset(
            buffer,
            0,
            FS_SECTOR_SIZE
        );



        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++, offset++)
        {
            buffer[i] =
                data[offset];
        }



        if(!disk_write(
            sector + s,
            buffer))
        {
            return false;
        }
    }



    file->size =
        size;



    fs_sync();



    return true;
}





/*
    Read file data

    Loads file contents
    from disk into memory.
*/

bool fs_read(
    char *name,
    char *buffer
)
{
    int index =
        fs_find(name);



    if(index < 0)
    {
        return false;
    }



    FileEntry *file =
        &fs.files[index];



    uint32_t size =
        file->size;



    uint32_t sectors =
        sectors_needed(size);



    uint32_t sector =
        file->start_sector;



    uint32_t offset = 0;



    uint8_t temp[FS_SECTOR_SIZE];



    for(uint32_t s = 0;
        s < sectors;
        s++)
    {

        if(!disk_read(
            sector + s,
            temp))
        {
            return false;
        }



        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++, offset++)
        {
            buffer[offset] =
                temp[i];
        }
    }



    buffer[size] = 0;


    return true;
}
/*
    Delete file

    Removes file entry
    from filesystem.
*/

bool fs_delete(char *name)
{
    int index =
        fs_find(name);



    if(index < 0)
    {
        return false;
    }



    FileEntry *file =
        &fs.files[index];



    memset(
        file,
        0,
        sizeof(FileEntry)
    );



    file->used =
        FILE_UNUSED;



    fs_sync();



    return true;
}





/*
    Get file size
*/

uint32_t fs_size(char *name)
{
    int index =
        fs_find(name);



    if(index < 0)
    {
        return 0;
    }



    return fs.files[index].size;
}





/*
    List files

    Used by ls command.
*/

void fs_list(void)
{
    for(int i = 0;
        i < FS_MAX_FILES;
        i++)
    {
        if(fs.files[i].used == FILE_USED)
        {
            print(
                fs.files[i].name
            );


            print("\n");
        }
    }
}
