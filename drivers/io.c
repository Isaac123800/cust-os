#include "io.h"

/*
    Basic port I/O helpers
    These wrap x86 IN/OUT instructions in safe inline assembly.
*/

void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

uint16_t inw(uint16_t port)
{
    uint16_t value;

    __asm__ volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

/*
    Block transfers (string I/O)
    Used by ATA/ATAPI for sector reads/writes.
*/

void insw(uint16_t port, void *buffer, uint32_t count)
{
    __asm__ volatile (
        "cld\n\t"
        "rep insw"
        : "+D"(buffer), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

void outsw(uint16_t port, const void *buffer, uint32_t count)
{
    __asm__ volatile (
        "cld\n\t"
        "rep outsw"
        : "+S"(buffer), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

/*
    io_wait:
    Sends a dummy OUT to port 0x80.
    This is a traditional delay used in old BIOS/ATA code.
*/

void io_wait(void)
{
    __asm__ volatile (
        "outb %%al, $0x80"
        :
        : "a"(0)
    );
}

