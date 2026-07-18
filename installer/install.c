#include "install.h"

#include "../fs/filesystem.h"


extern void print(char *text);



bool install_system(void)
{
    print("\nInstalling CustOS...\n\n");


    print("Formatting drive...\n");


    fs_format();



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



    fs_sync();



    print("\nInstallation complete!\n");


    return true;
}
