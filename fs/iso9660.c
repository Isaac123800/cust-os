#include "iso9660.h"

#include "../drivers/cdrom.h"


extern void print(char *text);



typedef struct
{
    uint8_t length;
    uint8_t ext_attr_length;

    uint32_t extent;
    uint32_t size;

    uint8_t date[7];

    uint8_t flags;

    uint8_t file_unit_size;
    uint8_t interleave;

    uint16_t volume_sequence;

    uint8_t name_length;

} DirectoryEntry;



static uint32_t root_sector = 0;
static uint32_t root_size = 0;



static bool string_equal(
    char *a,
    char *b
)
{
    while(*a && *b)
    {
        if(*a != *b)
            return false;

        a++;
        b++;
    }


    return *a == 0 && *b == 0;
}




void iso_init(void)
{
    uint8_t buffer[ISO_SECTOR_SIZE];


    /*
        Primary Volume Descriptor
        starts at sector 16
    */

    if(!cdrom_read_sector(
        16,
        buffer))
    {
        print("ISO read failed\n");
        return;
    }



    /*
        Check CD001
    */

    if(buffer[1] != 'C' ||
       buffer[2] != 'D' ||
       buffer[3] != '0' ||
       buffer[4] != '0' ||
       buffer[5] != '1')
    {
        print("Not ISO9660\n");
        return;
    }



    /*
        Root directory record starts at byte 156
    */

    DirectoryEntry *root =
        (DirectoryEntry *)(buffer + 156);



    root_sector =
        root->extent;



    root_size =
        root->size;



    print("ISO9660 detected\n");
}





bool iso_read_file(
    char *name,
    uint8_t *buffer,
    uint32_t *size
)
{
    if(root_sector == 0)
    {
        iso_init();

        if(root_sector == 0)
            return false;
    }



    static uint8_t sector[ISO_SECTOR_SIZE];



    uint32_t remaining =
        root_size;



    uint32_t current_sector =
        root_sector;



    while(remaining > 0)
    {

        if(!cdrom_read_sector(
            current_sector,
            sector))
        {
            return false;
        }



        uint32_t offset = 0;



        while(offset < ISO_SECTOR_SIZE)
        {
            DirectoryEntry *entry =
                (DirectoryEntry *)(sector + offset);



            if(entry->length == 0)
                break;



            char filename[128];



            uint8_t len =
                entry->name_length;



            if(len >= 127)
                len = 127;



            for(int i = 0; i < len; i++)
            {
                filename[i] =
                    sector[offset + 33 + i];
            }


            filename[len] = 0;



            /*
                ISO files often have ;1
                version suffix
            */


            for(int i = 0; filename[i]; i++)
            {
                if(filename[i] == ';')
                {
                    filename[i] = 0;
                    break;
                }
            }



            if(string_equal(
                filename,
                name))
            {

                uint32_t file_sector =
                    entry->extent;


                uint32_t file_size =
                    entry->size;



                *size = file_size;



                uint32_t sectors =
                    (file_size + ISO_SECTOR_SIZE - 1)
                    / ISO_SECTOR_SIZE;



                uint32_t copied = 0;



                for(uint32_t s = 0;
                    s < sectors;
                    s++)
                {
                    if(!cdrom_read_sector(
                        file_sector + s,
                        sector))
                    {
                        return false;
                    }



                    for(int i = 0;
                        i < ISO_SECTOR_SIZE &&
                        copied < file_size;
                        i++)
                    {
                        buffer[copied++] =
                            sector[i];
                    }
                }



                return true;
            }



            offset += entry->length;
        }



        current_sector++;

        if(remaining >= ISO_SECTOR_SIZE)
            remaining -= ISO_SECTOR_SIZE;
        else
            remaining = 0;
    }



    return false;
}
