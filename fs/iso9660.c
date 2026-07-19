#include "iso9660.h"

#include "../drivers/cdrom.h"


extern void print(char *text);



#define SECTOR_SIZE 2048



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



static uint32_t root_sector = 0;
static uint32_t root_size = 0;





static bool equal(
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


    return (*a == 0 && *b == 0);
}





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





static void copy_name(
    uint8_t *src,
    uint8_t len,
    char *dst
)
{
    if(len > 127)
        len = 127;


    for(int i = 0; i < len; i++)
        dst[i] = src[i];


    dst[len] = 0;
}






void iso_init(void)
{
    uint8_t buffer[SECTOR_SIZE];


    print("ISO: Reading PVD\n");



    if(!cdrom_read_sector(
        16,
        buffer))
    {
        print("ISO: PVD read failed\n");
        return;
    }



    if(buffer[0] != 1)
    {
        print("ISO: Wrong type\n");
        return;
    }



    if(buffer[1] != 'C' ||
       buffer[2] != 'D' ||
       buffer[3] != '0' ||
       buffer[4] != '0' ||
       buffer[5] != '1')
    {
        print("ISO: Signature failed\n");
        return;
    }



    DirectoryEntry *root =
        (DirectoryEntry *)(buffer + 156);



    root_sector = root->extent;

    root_size = root->size;



    print("ISO9660 detected\n");

}







void iso_list_root(void)
{

    if(root_sector == 0)
        iso_init();



    if(root_sector == 0)
    {
        print("ISO: No root\n");
        return;
    }




    uint8_t buffer[SECTOR_SIZE];



    uint32_t sector =
        root_sector;



    uint32_t remaining =
        root_size;



    print("ROOT DIRECTORY:\n");



    while(remaining > 0)
    {

        if(!cdrom_read_sector(
            sector,
            buffer))
        {
            print("ISO: directory read failed\n");
            return;
        }



        uint32_t offset = 0;



        while(offset < SECTOR_SIZE)
        {

            DirectoryEntry *entry =
                (DirectoryEntry *)(buffer + offset);



            if(entry->length == 0)
                break;



            char name[128];


            copy_name(
                buffer + offset + 33,
                entry->name_length,
                name
            );



            print(name);
            print("\n");



            offset += entry->length;
        }



        sector++;



        if(remaining >= SECTOR_SIZE)
            remaining -= SECTOR_SIZE;
        else
            remaining = 0;
    }
}







bool iso_read_file(
    char *wanted,
    uint8_t *buffer,
    uint32_t *size
)
{

    if(root_sector == 0)
        iso_init();



    if(root_sector == 0)
        return false;




    uint8_t sector_buffer[SECTOR_SIZE];



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
            print("ISO: directory sector failed\n");
            return false;
        }



        uint32_t offset = 0;



        while(offset < SECTOR_SIZE)
        {

            DirectoryEntry *entry =
                (DirectoryEntry *)(sector_buffer + offset);



            if(entry->length == 0)
                break;



            char name[128];



            copy_name(
                sector_buffer + offset + 33,
                entry->name_length,
                name
            );



            remove_version(name);



            if(equal(name, wanted))
            {

                print("ISO: File found\n");



                uint32_t file_sector =
                    entry->extent;



                uint32_t file_size =
                    entry->size;



                *size = file_size;



                uint32_t count =
                    (file_size + SECTOR_SIZE - 1)
                    /
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
                        print("ISO: file read failed\n");
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



            offset += entry->length;
        }



        sector++;



        if(remaining >= SECTOR_SIZE)
            remaining -= SECTOR_SIZE;
        else
            remaining = 0;
    }



    print("ISO: File not found\n");


    return false;
}
