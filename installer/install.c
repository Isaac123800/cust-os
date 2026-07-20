#include "install.h"

#include "../fs/filesystem.h"
#include "../fs/iso9660.h"

#include "../drivers/cdrom.h"
#include "../drivers/disk.h"


extern void print(char *text);



static uint8_t kernel_buffer[65536];



bool install_system(void)
{

    print("\nInstalling CustOS...\n\n");



    /*
        Initialize hard disk first
    */

    print("Initializing disk...\n");

    disk_init();



    /*
        Initialize installer CD
    */

    print("Initializing CD-ROM...\n");

    cdrom_init();




    /*
        Format target drive
    */

    print("Formatting drive...\n");


    if(!fs_format())
    {
        print("Format failed\n");
        return false;
    }


    print("Format complete\n");




    /*
        Show ISO contents
    */

    print("Listing ISO...\n");

    iso_list_root();




    /*
        Load kernel from installer CD
    */

    print("\nReading KERNEL.BIN...\n");


    uint32_t size = 0;



    if(!iso_read_file(
        "KERNEL.BIN",
        kernel_buffer,
        &size))
    {
        print("Could not read kernel.bin\n");

        return false;
    }



    print("Kernel loaded\n");




    /*
        Create kernel file
    */

    if(!fs_create("KERNEL.BIN"))
    {
        print("Create failed\n");

        return false;
    }




    /*
        Write kernel to HDD
    */

    if(!fs_write(
        "KERNEL.BIN",
        (char *)kernel_buffer,
        size))
    {
        print("Write failed\n");

        return false;
    }



    print("Kernel copied\n");



    /*
        Save filesystem changes
    */

    fs_sync();



    print("\nInstallation complete\n");


    return true;
}
