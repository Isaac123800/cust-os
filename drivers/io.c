#include "io.h"


// ============================================================
// OUTPUT BYTE
// ============================================================

void outb(
    uint16_t port,
    uint8_t value
)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value),
          "Nd"(port)
    );
}


// ============================================================
// INPUT BYTE
// ============================================================

uint8_t inb(
    uint16_t port
)
{
    uint8_t value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}


// ============================================================
// OUTPUT WORD
// ============================================================

void outw(
    uint16_t port,
    uint16_t value
)
{
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a"(value),
          "Nd"(port)
    );
}


// ============================================================
// INPUT WORD
// ============================================================

uint16_t inw(
    uint16_t port
)
{
    uint16_t value;

    __asm__ volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}


// ============================================================
// INPUT WORD STRING
// ============================================================

void insw(
    uint16_t port,
    void *buffer,
    uint32_t count
)
{
    __asm__ volatile (
        "cld\n\t"
        "rep insw"
        : "+D"(buffer),
          "+c"(count)
        : "d"(port)
        : "memory"
    );
}


// ============================================================
// OUTPUT WORD STRING
//
// Words are sent individually rather than using REP OUTSW.
// This is used by the ATA/ATAPI code.
// ============================================================

void outsw(
    uint16_t port,
    const void *buffer,
    uint32_t count
)
{
    const uint16_t *words =
        (const uint16_t *)buffer;

    for(uint32_t i = 0;
        i < count;
        i++)
    {
        outw(
            port,
            words[i]
        );

        io_wait();
    }
}


// ============================================================
// I/O WAIT
// ============================================================

void io_wait(void)
{
    __asm__ volatile (
        "outb %%al, $0x80"
        :
        : "a"(0)
    );
}

