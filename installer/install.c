#include "install.h"

#include "../fs/filesystem.h"
#include "../fs/iso9660.h"


extern void print(char *text);



static uint8_t kernel_buffer[65536];



bool install_system(void)
{

    print("\nInstalling CustOS...\n\n");



    print("Formatting drive...\n");


    fs_format();



    print("Format complete\n");



    print("Listing ISO...\n");


    iso_list_root();



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



    if(!fs_create("KERNEL.BIN"))
    {
        print("Create failed\n");

        return false;
    }



    if(!fs_write(
        "KERNEL.BIN",
        (char *)kernel_buffer,
        size))
    {
        print("Write failed\n");

        return false;
    }



    print("Kernel copied\n");



    fs_sync();



    print("\nInstallation complete\n");


    return true;
}
