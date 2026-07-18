#include "install.h"

#include "grub_install.h"

#include "../fs/filesystem.h"
#include "../fs/iso9660.h"


extern void print(char *text);



bool install_system(void)
{
    print("\nInstalling CustOS...\n\n");


    /*
        Format target disk
    */

    print("Formatting drive...\n");


    fs_format();


    print("Format complete\n\n");



    /*
        Install bootloader
    */

    print("Installing bootloader...\n");


    if(!install_grub())
    {
        print("Bootloader installation failed\n");

        return false;
    }


    print("Bootloader installed\n\n");



    /*
        Read kernel from ISO
    */

    print("Reading KERNEL.BIN from ISO...\n");


    uint8_t kernel_buffer[65536];


    uint32_t kernel_size = 0;



    if(!iso_read_file(
        "KERNEL.BIN",
        kernel_buffer,
        &kernel_size))
    {
        print("Could not read KERNEL.BIN\n");

        return false;
    }



    print("Kernel loaded from ISO\n\n");



    /*
        Create kernel file
    */

    print("Creating KERNEL.BIN...\n");


    if(!fs_create("KERNEL.BIN"))
    {
        print("Failed creating KERNEL.BIN\n");

        return false;
    }



    print("Writing KERNEL.BIN...\n");


    if(!fs_write(
        "KERNEL.BIN",
        (char *)kernel_buffer,
        kernel_size))
    {
        print("Failed writing KERNEL.BIN\n");

        return false;
    }


    print("Kernel copied\n\n");



    /*
        Create system files
    */

    print("Creating system files...\n");



    if(!fs_create("SYSTEM.BIN"))
    {
        print("Failed creating SYSTEM.BIN\n");

        return false;
    }



    if(!fs_create("CONFIG.TXT"))
    {
        print("Failed creating CONFIG.TXT\n");

        return false;
    }



    print("Syncing filesystem...\n");


    fs_sync();



    print("\nCustOS installation complete!\n");

    print("Restart to boot from disk.\n");



    return true;
}
