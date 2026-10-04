#include "install.h"

#include "../fs/filesystem.h"
#include "../fs/iso9660.h"

#include "../drivers/cdrom.h"
#include "../drivers/disk.h"

extern void print(char *text);
extern void print_hex(uint8_t value);


/*
    ============================================================
    INSTALLER BUFFERS
    ============================================================
*/

static uint8_t kernel_buffer[65536];
static uint8_t bootloader_buffer[512];


/*
    ============================================================
    INSTALL SYSTEM
    ============================================================
*/

bool install_system(void)
{
    print("\nInstalling CustOS...\n\n");


    /*
        --------------------------------------------------------
        Initialise target disk
        --------------------------------------------------------
    */

    print("Initializing disk...\n");

    disk_init();


    /*
        --------------------------------------------------------
        Initialise CD-ROM
        --------------------------------------------------------
    */

    print("Initializing CD-ROM...\n");

    cdrom_init();


    /*
        --------------------------------------------------------
        Initialise ISO filesystem
        --------------------------------------------------------
    */

    print("Initializing ISO...\n");

    iso_init();


    /*
        --------------------------------------------------------
        Show ISO contents
        --------------------------------------------------------
    */

    print("\nListing ISO...\n");

    iso_list_root();


    /*
        --------------------------------------------------------
        Read bootloader from ISO
        --------------------------------------------------------
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
        --------------------------------------------------------
        Verify bootloader size
        --------------------------------------------------------
    */

    if(bootloader_size != 512)
    {
        print("Invalid bootloader size\n");
        return false;
    }


    /*
        --------------------------------------------------------
        IMPORTANT:
        Verify that the bootloader we read from the ISO
        actually contains the BIOS boot signature.
        --------------------------------------------------------
    */

    print("Checking ISO bootloader: ");

    print_hex(bootloader_buffer[510]);
    print(" ");

    print_hex(bootloader_buffer[511]);

    print("\n");


    if(bootloader_buffer[510] != 0x55 ||
       bootloader_buffer[511] != 0xAA)
    {
        print("ISO BOOTLOADER INVALID\n");
        return false;
    }

    print("ISO bootloader verified\n");


    /*
        --------------------------------------------------------
        Format target disk
        --------------------------------------------------------
    */

    print("Formatting drive...\n");

    if(!fs_format())
    {
        print("Format failed\n");
        return false;
    }

    print("Format complete\n");


    /*
        --------------------------------------------------------
        Install bootloader into sector 0
        --------------------------------------------------------
    */

    print("Installing bootloader...\n");

    if(!disk_write(
        0,
        bootloader_buffer))
    {
        print("Could not install bootloader\n");
        return false;
    }

    print("Bootloader installed\n");


    /*
        --------------------------------------------------------
        Read sector 0 back immediately
        --------------------------------------------------------
    */

    uint8_t test_sector[512];

    print("Checking bootloader on disk...\n");

    if(!disk_read(
        0,
        test_sector))
    {
        print("Could not read bootloader back\n");
        return false;
    }


    /*
        --------------------------------------------------------
        Show what was actually read from sector 0
        --------------------------------------------------------
    */

    print("DISK READBACK: ");

    print_hex(test_sector[0]);
    print(" ");

    print_hex(test_sector[1]);
    print(" ");

    print_hex(test_sector[2]);
    print(" ");

    print_hex(test_sector[3]);
    print(" ... ");

    print_hex(test_sector[510]);
    print(" ");

    print_hex(test_sector[511]);

    print("\n");


    /*
        --------------------------------------------------------
        Verify boot signature
        --------------------------------------------------------
    */

    if(test_sector[510] == 0x55 &&
       test_sector[511] == 0xAA)
    {
        print("Bootloader verified on disk!\n");
    }
    else
    {
        print("BOOTLOADER DISK VERIFY FAILED\n");
        return false;
    }


    /*
        --------------------------------------------------------
        Read kernel from ISO
        --------------------------------------------------------
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
        --------------------------------------------------------
        Install raw kernel
        --------------------------------------------------------
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
        --------------------------------------------------------
        Create filesystem kernel file
        --------------------------------------------------------
    */

    print("Creating kernel file...\n");

    if(!fs_create("KERNEL.BIN"))
    {
        print("Create failed\n");
        return false;
    }


    /*
        --------------------------------------------------------
        Write kernel into filesystem
        --------------------------------------------------------
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
        --------------------------------------------------------
        Synchronise filesystem
        --------------------------------------------------------
    */

    fs_sync();


    /*
        --------------------------------------------------------
        Finished
        --------------------------------------------------------
    */

    print("\nInstallation complete\n");

    return true;
}
