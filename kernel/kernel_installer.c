// CustOS installer kernel
// This kernel is loaded by GRUB from the ISO.
// It is NOT copied to the installed disk.

#include <stdbool.h>

#include "include/types.h"
#include "include/console.h"

#include "drivers/disk.h"
#include "drivers/cdrom.h"

#include "fs/iso9660.h"

#include "installer/installer.h"


// ============================================================
// INSTALLER KERNEL ENTRY
// ============================================================

void kernel_installer_main()
{
    clear();

    print(
        "Starting CustOS Installer...\n\n"
    );

    print(
        "Initializing disk...\n"
    );

    disk_init();

    print(
        "Disk ready\n\n"
    );

    print(
        "Initializing CD-ROM...\n"
    );

    cdrom_init();

    print(
        "\nTesting ISO...\n"
    );

    iso_init();

    print(
        "\nStarting installer...\n"
    );

    installer_start();

    print(
        "\nInstaller finished.\n"
    );

    while(1)
    {
    }
}

