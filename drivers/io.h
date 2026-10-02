#ifndef IO_H
#define IO_H

#include "../include/types.h"


// ============================================================
// 8-BIT I/O
// ============================================================

/* Write an 8-bit value to an I/O port */
void outb(uint16_t port, uint8_t value);

/* Read an 8-bit value from an I/O port */
uint8_t inb(uint16_t port);


// ============================================================
// 16-BIT I/O
// ============================================================

/* Write a 16-bit value to an I/O port */
void outw(uint16_t port, uint16_t value);

/* Read a 16-bit value from an I/O port */
uint16_t inw(uint16_t port);


// ============================================================
// STRING I/O
// ============================================================

/*
 * Read 'count' 16-bit words from an I/O port.
 *
 * The destination buffer must contain enough space for:
 *
 *     count * 2 bytes
 */
void insw(
    uint16_t port,
    void *buffer,
    uint32_t count
);


/*
 * Write 'count' 16-bit words to an I/O port.
 *
 * The source buffer must contain at least:
 *
 *     count * 2 bytes
 */
void outsw(
    uint16_t port,
    const void *buffer,
    uint32_t count
);


// ============================================================
// I/O WAIT
// ============================================================

/*
 * Small hardware I/O delay.
 *
 * Normally implemented using port 0x80 on x86.
 */
void io_wait(void);


#endif
