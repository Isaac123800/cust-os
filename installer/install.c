#include "install.h"

#include "grub_install.h"

#include "../fs/filesystem.h"
#include "../fs/iso9660.h"


extern void print(char *text);



bool install_system(void)
{
    print("\nInstalling CustOS...\n\n");



    /*
        Format target drive
    */

    print("Formatting drive...\n");


    fs_format();



    /*
        Install bootloader
    */

    print("Installing bootloader...\n");


    if(!install_grub())
    {
        print("Bootloader installation failed\n");

        return false;
    }



    /*
        Copy kernel from ISO
    */

    print("Copying kernel...\n");



    uint8_t kernel_buffer[65536];

    uint32_t kernel_size = 0;



    if(!iso_read_file(
            "KERNEL.BIN",
            kernel_buffer,
            &kernel_size))
    {
        print("Could not find KERNEL.BIN on installer media\n");

        return false;
    }



    /*
        Create kernel file
    */

    if(!fs_create("KERNEL.BIN"))
    {
        print("Could not create KERNEL.BIN\n");

        return false;
    }



    /*
        Write kernel data
    */

    if(!fs_write(
            "KERNEL.BIN",
            (char *)kernel_buffer,
            kernel_size))
    {
        print("Could not write KERNEL.BIN\n");

        return false;
    }



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



    /*
        Save filesystem changes
    */

    fs_sync();



    print("\nCustOS installation complete!\n");

    print("Restart the computer to boot CustOS.\n");



    return true;
}
