#include "iso9660.h"

#include "../drivers/cdrom.h"
#include "../include/console.h"


#define SECTOR_SIZE 2048


// ============================================================
// ISO9660 DIRECTORY ENTRY
// ============================================================

typedef struct __attribute__((packed))
{
    uint8_t length;

    uint8_t ext_attr_length;

    uint32_t extent;
    uint32_t extent_be;

    uint32_t size;
    uint32_t size_be;

    uint8_t date[7];

    uint8_t flags;

    uint8_t file_unit_size;
    uint8_t interleave;

    uint16_t volume_sequence;
    uint16_t volume_sequence_be;

    uint8_t name_length;

} DirectoryEntry;


// ============================================================
// ISO STATE
// ============================================================

static uint32_t root_sector = 0;

static uint32_t root_size = 0;


// ============================================================
// READ LITTLE-ENDIAN 32-BIT VALUE
// ============================================================

static uint32_t read_le32(
    uint8_t *p
)
{
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}


// ============================================================
// STRING EQUAL
// ============================================================

static bool equal(
    char *a,
    char *b
)
{
    while(*a &&
          *b)
    {
        if(*a != *b)
        {
            return false;
        }

        a++;
        b++;
    }

    return (
        *a == 0 &&
        *b == 0
    );
}


// ============================================================
// REMOVE ISO9660 VERSION
//
// Example:
//
// KERNEL.BIN;1
//
// becomes:
//
// KERNEL.BIN
// ============================================================

static void remove_version(
    char *name
)
{
    while(*name)
    {
        if(*name == ';')
        {
            *name = 0;
            return;
        }

        name++;
    }
}


// ============================================================
// COPY ISO NAME
// ============================================================

static void copy_name(
    uint8_t *src,
    uint8_t len,
    char *dst
)
{
    if(len > 127)
    {
        len = 127;
    }

    for(uint8_t i = 0;
        i < len;
        i++)
    {
        dst[i] =
            src[i];
    }

    dst[len] = 0;
}


// ============================================================
// INITIALISE ISO9660
// ============================================================

void iso_init(void)
{
    uint8_t buffer[
        SECTOR_SIZE
    ];


    print(
        "ISO: Reading PVD\n"
    );


    /*
        ISO9660 Primary Volume Descriptor
        is located at sector 16.
    */

    if(!cdrom_read_sector(
        16,
        buffer))
    {
        print(
            "ISO: PVD read failed\n"
        );

        return;
    }


    /*
        PVD type must be 1.
    */

    if(buffer[0] != 1)
    {
        print(
            "ISO: Wrong type\n"
        );

        return;
    }


    /*
        ISO9660 identifier is "CD001".
    */

    if(buffer[1] != 'C' ||
       buffer[2] != 'D' ||
       buffer[3] != '0' ||
       buffer[4] != '0' ||
       buffer[5] != '1')
    {
        print(
            "ISO: Signature failed\n"
        );

        return;
    }


    /*
        Root directory record starts
        at byte 156.
    */

    DirectoryEntry *root =
        (DirectoryEntry *)
        (buffer + 156);


    root_sector =
        read_le32(
            (uint8_t *)&root->extent
        );


    root_size =
        read_le32(
            (uint8_t *)&root->size
        );


    print(
        "ISO9660 detected\n"
    );
}


// ============================================================
// LIST ROOT DIRECTORY
// ============================================================

void iso_list_root(void)
{
    if(root_sector == 0)
    {
        iso_init();
    }


    if(root_sector == 0)
    {
        print(
            "ISO: No root\n"
        );

        return;
    }


    uint8_t buffer[
        SECTOR_SIZE
    ];


    uint32_t sector =
        root_sector;

    uint32_t remaining =
        root_size;


    print(
        "ROOT DIRECTORY:\n"
    );


    while(remaining > 0)
    {
        if(!cdrom_read_sector(
            sector,
            buffer))
        {
            print(
                "ISO: directory read failed\n"
            );

            return;
        }


        uint32_t offset = 0;


        while(offset < SECTOR_SIZE)
        {
            DirectoryEntry *entry =
                (DirectoryEntry *)
                (buffer + offset);


            /*
                Zero-length entry means
                the rest of the sector is unused.
            */

            if(entry->length == 0)
            {
                break;
            }


            /*
                Minimum ISO9660 directory
                record length.
            */

            if(entry->length < 34)
            {
                break;
            }


            /*
                Prevent an entry from
                crossing the sector.
            */

            if(offset + entry->length >
               SECTOR_SIZE)
            {
                break;
            }


            char name[128];


            uint8_t name_length =
                entry->name_length;


            if(name_length >
               entry->length - 33)
            {
                name_length =
                    entry->length - 33;
            }


            copy_name(
                buffer + offset + 33,
                name_length,
                name
            );


            remove_version(name);


            /*
                ISO9660 uses:
                0 = "."
                1 = ".."
            */

            if(name_length == 1 &&
               (name[0] == 0 ||
                name[0] == 1))
            {
                offset +=
                    entry->length;

                continue;
            }


            print(name);

            print("\n");


            offset +=
                entry->length;
        }


        sector++;


        if(remaining >= SECTOR_SIZE)
        {
            remaining -=
                SECTOR_SIZE;
        }
        else
        {
            remaining = 0;
        }
    }
}


// ============================================================
// READ FILE FROM ISO
// ============================================================

bool iso_read_file(
    char *wanted,
    uint8_t *buffer,
    uint32_t *size
)
{
    if(wanted == 0 ||
       buffer == 0 ||
       size == 0)
    {
        return false;
    }


    if(root_sector == 0)
    {
        iso_init();
    }


    if(root_sector == 0)
    {
        return false;
    }


    uint8_t sector_buffer[
        SECTOR_SIZE
    ];


    uint32_t sector =
        root_sector;

    uint32_t remaining =
        root_size;


    while(remaining > 0)
    {
        if(!cdrom_read_sector(
            sector,
            sector_buffer))
        {
            print(
                "ISO: directory sector failed\n"
            );

            return false;
        }


        uint32_t offset = 0;


        while(offset < SECTOR_SIZE)
        {
            DirectoryEntry *entry =
                (DirectoryEntry *)
                (sector_buffer + offset);


            if(entry->length == 0)
            {
                break;
            }


            if(entry->length < 34)
            {
                break;
            }


            if(offset + entry->length >
               SECTOR_SIZE)
            {
                break;
            }


            char name[128];


            uint8_t name_length =
                entry->name_length;


            if(name_length >
               entry->length - 33)
            {
                name_length =
                    entry->length - 33;
            }


            copy_name(
                sector_buffer + offset + 33,
                name_length,
                name
            );


            remove_version(name);


            if(equal(
                name,
                wanted))
            {
                /*
                    Do not try to read
                    directories as files.
                */

                if(entry->flags & 0x02)
                {
                    print(
                        "ISO: Requested path is a directory\n"
                    );

                    return false;
                }


                print(
                    "ISO: File found\n"
                );


                uint32_t file_sector =
                    read_le32(
                        (uint8_t *)&entry->extent
                    );


                uint32_t file_size =
                    read_le32(
                        (uint8_t *)&entry->size
                    );


                *size =
                    file_size;


                uint32_t count =
                    (file_size +
                     SECTOR_SIZE -
                     1) /
                    SECTOR_SIZE;


                uint32_t copied = 0;


                for(uint32_t i = 0;
                    i < count;
                    i++)
                {
                    if(!cdrom_read_sector(
                        file_sector + i,
                        sector_buffer))
                    {
                        print(
                            "ISO: file read failed\n"
                        );

                        return false;
                    }


                    for(uint32_t x = 0;
                        x < SECTOR_SIZE &&
                        copied < file_size;
                        x++)
                    {
                        buffer[copied++] =
                            sector_buffer[x];
                    }
                }


                return true;
            }


            offset +=
                entry->length;
        }


        sector++;


        if(remaining >= SECTOR_SIZE)
        {
            remaining -=
                SECTOR_SIZE;
        }
        else
        {
            remaining = 0;
        }
    }


    print(
        "ISO: File not found\n"
    );


    return false;
}

