#include "install.h"

#include "../fs/filesystem.h"
#include "../fs/iso9660.h"

#include "../drivers/cdrom.h"
#include "../drivers/disk.h"

extern void print(char *text);

static uint8_t kernel_buffer[65536];
static uint8_t bootloader_buffer[512];


bool install_system(void)
{
    print("\nInstalling CustOS...\n\n");


    /*
        Initialize hard disk
    */

    print("Initializing disk...\n");

    disk_init();


    /*
        Initialize installer CD
    */

    print("Initializing CD-ROM...\n");

    cdrom_init();


    /*
        Initialize ISO9660
    */

    print("Initializing ISO...\n");

    iso_init();


    /*
        DEBUG:
        List everything in the ISO root directory
        before trying to read any files.
    */

    print("\nListing ISO...\n");

    iso_list_root();


    /*
        Read bootloader from ISO
    */

    print("\nReading bootloader...\n");

    uint32_t bootloader_size = 0;

    if(!iso_read_file(
        "BOOTLOAD.BIN",
        bootloader_buffer,
        &bootloader_size))
    {
        print("Could not read bootloader.bin\n");
        return false;
    }

    print("Bootloader loaded\n");


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
        Install bootloader to sector 0
    */

    print("Installing bootloader...\n");

    if(bootloader_size != 512)
    {
        print("Invalid bootloader size\n");
        return false;
    }

    if(!disk_write(
        0,
        bootloader_buffer))

        print("READBACK: ");
        
        print_hex(test_sector[3]);
        print(" ");
        
        print(" ... ");
        
        print_hex(test_sector[0]);
        print(" ");
        
        print_hex(test_sector[1]);
        print(" ");
        
        print_hex(test_sector[2]);
        print(" ");
        
        print_hex(test_sector[510]);
        print(" ");
        
        print_hex(test_sector[511]);
        
        print("\n");
    {
        print("Could not install bootloader\n");
        return false;
    }

    print("Bootloader installed\n");

    uint8_t test_sector[512];

    print("Checking bootloader...\n");
    
    if(!disk_read(0, test_sector))
    {
        print("Could not read bootloader back\n");
        return false;
    }
    
    if(test_sector[510] == 0x55 &&
       test_sector[511] == 0xAA)
    {
        print("Bootloader verified!\n");
    }
    else
    {
        print("BOOTLOADER VERIFY FAILED\n");
    }
    
    /*
        Read kernel from ISO
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
        Install raw kernel.
        
        Sector 0       = bootloader
        Sectors 1-38   = raw kernel
        Sector 39+     = filesystem
    */

    print("Installing kernel...\n");

    uint32_t kernel_sectors =
        (size + 511) / 512;

    for(uint32_t i = 0;
        i < kernel_sectors;
        i++)
    {
        if(!disk_write(
            1 + i,
            kernel_buffer + (i * 512)))
        {
            print("Could not install kernel\n");
            return false;
        }
    }

    print("Kernel installed\n");


    /*
        Also create KERNEL.BIN
        inside the CustOS filesystem
    */

    print("Creating kernel file...\n");

    if(!fs_create("KERNEL.BIN"))
    {
        print("Create failed\n");
        return false;
    }


    /*
        Write kernel to filesystem
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
