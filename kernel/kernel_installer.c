// CustOS installer kernel
// This kernel is loaded by GRUB from the ISO.
// It is NOT copied to the installed disk.

#include <stdbool.h>

#include "include/types.h"

#include "drivers/disk.h"
#include "drivers/cdrom.h"

#include "fs/iso9660.h"

#include "installer/installer.h"


// ============================================================
// VIDEO
// ============================================================

#define VIDEO_MEMORY 0xB8000

char *video =
    (char *)VIDEO_MEMORY;

int cursor = 0;

int text_color = 7;
int bg_color = 0;


// ============================================================
// SCROLL
// ============================================================

void scroll()
{
    for(int y = 1; y < 25; y++)
    {
        for(int x = 0; x < 80; x++)
        {
            video[((y - 1) * 80 + x) * 2] =
                video[(y * 80 + x) * 2];

            video[((y - 1) * 80 + x) * 2 + 1] =
                video[(y * 80 + x) * 2 + 1];
        }
    }

    for(int x = 0; x < 80; x++)
    {
        video[(24 * 80 + x) * 2] = ' ';

        video[(24 * 80 + x) * 2 + 1] =
            (bg_color << 4) | text_color;
    }

    cursor = 24 * 80;
}


// ============================================================
// PUTCHAR
// ============================================================

void putchar(char c)
{
    if(c == '\n')
    {
        cursor =
            ((cursor / 80) + 1) * 80;
    }
    else
    {
        video[cursor * 2] = c;

        video[cursor * 2 + 1] =
            (bg_color << 4) | text_color;

        cursor++;
    }

    if(cursor >= 80 * 25)
    {
        scroll();
    }
}


// ============================================================
// PRINT
// ============================================================

void print(char *text)
{
    while(*text)
    {
        putchar(*text);
        text++;
    }
}


// ============================================================
// PRINT HEX
// ============================================================

void print_hex(uint8_t value)
{
    char hex[] =
        "0123456789ABCDEF";

    char out[3];

    out[0] =
        hex[(value >> 4) & 0xF];

    out[1] =
        hex[value & 0xF];

    out[2] = 0;

    print(out);
}


// ============================================================
// CLEAR
// ============================================================

void clear()
{
    for(int i = 0; i < 80 * 25; i++)
    {
        video[i * 2] = ' ';

        video[i * 2 + 1] =
            (bg_color << 4) | text_color;
    }

    cursor = 0;
}


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

