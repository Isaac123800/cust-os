#include "install.h"

#include "grub_install.h"

#include "../fs/filesystem.h"


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

    if(!install_grub())
    {
        print("Bootloader installation failed\n");

        return false;
    }



    /*
        Create system files
    */

    print("Creating system files...\n");



    if(!fs_create("KERNEL.BIN"))
    {
        print("Failed creating kernel file\n");

        return false;
    }



    if(!fs_create("SYSTEM.BIN"))
    {
        print("Failed creating system file\n");

        return false;
    }



    if(!fs_create("CONFIG.TXT"))
    {
        print("Failed creating config file\n");

        return false;
    }



    /*
        Save filesystem
    */

    fs_sync();



    print("\nCustOS installation complete!\n");


    print("You may restart the computer.\n");



    return true;
}
