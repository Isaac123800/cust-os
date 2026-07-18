#include "iso9660.h"

#include "../drivers/cdrom.h"



extern void print(char *text);



/*
    ISO9660 is initialized here
*/

void iso_init(void)
{
    print("ISO9660 initialized\n");
}



/*
    Read file from ISO

    Temporary version.

    Next step:
    - Read Primary Volume Descriptor
    - Read directory records
    - Locate file
    - Read sectors
*/

bool iso_read_file(
    char *name,
    uint8_t *buffer,
    uint32_t *size
)
{

    print("Reading ISO file: ");

    print(name);

    print("\n");


    /*
        Not implemented yet
    */


    return false;
}
