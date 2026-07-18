#include "grub_install.h"

#include "../drivers/disk.h"


extern void print(char *text);



/*
    Temporary GRUB installer

    This currently checks that the
    disk can be written.

    Real GRUB core image writing
    will be added after we add
    ISO file reading.
*/


bool install_grub(void)
{
    uint8_t buffer[512];


    print("Installing GRUB...\n");


    /*
        Test writing sector 1.

        Later this will contain:
        - boot.img
        - core.img
    */


    for(int i = 0; i < 512; i++)
    {
        buffer[i] = 0;
    }



    if(!disk_write(1, buffer))
    {
        print("GRUB install failed\n");

        return false;
    }



    print("GRUB stage written\n");


    return true;
}
