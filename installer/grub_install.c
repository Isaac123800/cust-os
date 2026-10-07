#include "grub_install.h"

#include "../drivers/disk.h"
#include "../include/console.h"


/*
    GRUB installation

    CustOS currently uses its own BIOS bootloader.

    Therefore GRUB must NOT write to the installed
    disk yet.

    In particular, LBA 1 contains the first sector
    of the installed CustOS kernel, so writing there
    would corrupt the kernel.
*/


bool install_grub(void)
{
    print(
        "GRUB installation is not required.\n"
    );

    print(
        "Using the CustOS bootloader.\n"
    );

    return true;
}

