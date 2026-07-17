#include "filesystem.h"

#include "../drivers/disk.h"


/*
    Global filesystem
*/

FileSystem fs;


/*
    Next free data sector

    Tracks the next available
    location for file data.
*/

static uint32_t next_free_sector = FS_DATA_START;



/*
    External terminal output

    Provided by kernel.
*/

extern void print(char *text);



/*
    Internal memory helpers
*/


static void memset(void *ptr, uint8_t value, uint32_t size)
{
    uint8_t *p = (uint8_t *)ptr;


    for(uint32_t i = 0; i < size; i++)
    {
        p[i] = value;
    }
}




/*
    Safe string copy

    Prevents filename overflow.
*/

static void strcpy_safe(
    char *dest,
    char *src,
    uint32_t max
)
{
    uint32_t i = 0;


    while(src[i] && i < max - 1)
    {
        dest[i] = src[i];

        i++;
    }


    dest[i] = 0;
}





/*
    Compare two strings
*/

static bool string_equal(
    char *a,
    char *b
)
{
    uint32_t i = 0;


    while(a[i] && b[i])
    {
        if(a[i] != b[i])
        {
            return false;
        }


        i++;
    }


    return a[i] == b[i];
}





/*
    Calculate sectors needed
*/

static uint32_t sectors_needed(
    uint32_t size
)
{
    if(size == 0)
    {
        return 1;
    }


    return (size + FS_SECTOR_SIZE - 1)
            / FS_SECTOR_SIZE;
}
/*
    Save superblock
*/

static void save_superblock(void)
{
    uint8_t buffer[FS_SECTOR_SIZE];


    memset(
        buffer,
        0,
        FS_SECTOR_SIZE
    );


    /*
        Safety check
    */

    if(sizeof(SuperBlock) > FS_SECTOR_SIZE)
    {
        return;
    }



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

static bool load_superblock(void)
{
    uint8_t buffer[FS_SECTOR_SIZE];



    if(!disk_read(
        FS_SUPERBLOCK_SECTOR,
        buffer))
    {
        return false;
    }



    if(sizeof(SuperBlock) > FS_SECTOR_SIZE)
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
    Save file table
*/

static void save_file_table(void)
{
    uint8_t buffer[FS_SECTOR_SIZE];


    uint8_t *table =
        (uint8_t *)fs.files;



    uint32_t size =
        sizeof(fs.files);



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
            buffer[i] =
                table[offset];
        }



        if(!disk_write(
            sector,
            buffer))
        {
            return;
        }


        sector++;
    }
}





/*
    Load file table
*/

static bool load_file_table(void)
{
    uint8_t buffer[FS_SECTOR_SIZE];


    uint8_t *table =
        (uint8_t *)fs.files;



    uint32_t size =
        sizeof(fs.files);



    uint32_t offset = 0;



    uint32_t sector =
        FS_FILE_TABLE_START;



    while(offset < size)
    {
        if(!disk_read(
            sector,
            buffer))
        {
            return false;
        }



        for(uint32_t i = 0;
            i < FS_SECTOR_SIZE &&
            offset < size;
            i++, offset++)
        {
            table[offset] =
                buffer[i];
        }



        sector++;
    }



    return true;
}
    /*
    Format filesystem

    Creates a fresh filesystem.
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
        disk_sector_count();



    fs.super.file_table_start =
        FS_FILE_TABLE_START;



    fs.super.data_start =
        FS_DATA_START;



    fs.super.free_blocks =
        fs.super.total_sectors -
        FS_DATA_START;



    next_free_sector =
        FS_DATA_START;



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



    if(!load_file_table())
    {
        return false;
    }



    next_free_sector =
        fs.super.data_start;



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
    {
        return;
    }



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
    if(name == 0)
    {
        return -1;
    }



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

static int find_free_entry(void)
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
    Allocate sectors

    Returns the first sector
    of the allocated block.
*/

static uint32_t allocate_sectors(
    uint32_t count
)
{
    if(count == 0)
    {
        return 0;
    }



    if(next_free_sector + count >
       fs.super.total_sectors)
    {
        return 0;
    }



    uint32_t start =
        next_free_sector;



    next_free_sector += count;



    if(fs.super.free_blocks >= count)
    {
        fs.super.free_blocks -= count;
    }
    else
    {
        fs.super.free_blocks = 0;
    }



    return start;
}/*
    Create file

    Equivalent to:
    touch filename
*/

bool fs_create(char *name)
{
    if(name == 0)
    {
        return false;
    }



    /*
        Do not allow duplicate files
    */

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



    strcpy_safe(
        file->name,
        name,
        FS_FILENAME_LENGTH
    );



    file->used =
        FILE_USED;



    file->size =
        0;



    /*
        Reserve one sector initially.

        This gives empty files
        a valid disk location.
    */

    file->start_sector =
        allocate_sectors(1);



    if(file->start_sector == 0)
    {
        file->used =
            FILE_UNUSED;

        return false;
    }



    fs_sync();



    return true;
}
/*
    Write data to file
*/

bool fs_write(
    char *name,
    char *data,
    uint32_t size
)
{
    if(name == 0 || data == 0)
    {
        return false;
    }



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



    /*
        Allocate enough space
        for the file.
    */

    uint32_t start =
        allocate_sectors(sectors);



    if(start == 0)
    {
        return false;
    }



    file->start_sector =
        start;



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
            start + s,
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
    if(name == 0 || buffer == 0)
    {
        return false;
    }



    int index =
        fs_find(name);



    if(index < 0)
    {
        return false;
    }



    FileEntry *file =
        &fs.files[index];



    if(file->used != FILE_USED)
    {
        return false;
    }



    uint32_t size =
        file->size;



    if(size > FS_MAX_FILE_SIZE)
    {
        return false;
    }



    uint32_t sectors =
        sectors_needed(size);



    uint32_t offset = 0;



    uint8_t temp[FS_SECTOR_SIZE];



    for(uint32_t s = 0;
        s < sectors;
        s++)
    {
        if(!disk_read(
            file->start_sector + s,
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
    if(name == 0)
    {
        return false;
    }



    int index =
        fs_find(name);



    if(index < 0)
    {
        return false;
    }



    memset(
        &fs.files[index],
        0,
        sizeof(FileEntry)
    );



    fs.files[index].used =
        FILE_UNUSED;



    fs_sync();



    return true;
}





/*
    Get file size
*/

uint32_t fs_size(char *name)
{
    if(name == 0)
    {
        return 0;
    }



    int index =
        fs_find(name);



    if(index < 0)
    {
        return 0;
    }



    if(fs.files[index].used != FILE_USED)
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
    bool found = false;


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


            found = true;
        }
    }



    if(!found)
    {
        print("No files\n");
    }
}



    return true;
}
