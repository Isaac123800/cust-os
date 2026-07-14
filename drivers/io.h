#ifndef IO_H
#define IO_H

#include "../include/types.h"

/* Write 8 bits to an I/O port */
void outb(uint16_t port, uint8_t value);

/* Read 8 bits from an I/O port */
uint8_t inb(uint16_t port);

/* Write 16 bits to an I/O port */
void outw(uint16_t port, uint16_t value);

/* Read 16 bits from an I/O port */
uint16_t inw(uint16_t port);

/* Read multiple 16-bit words from a port */
void insw(uint16_t port, void *buffer, uint32_t count);

/* Write multiple 16-bit words to a port */
void outsw(uint16_t port, const void *buffer, uint32_t count);

/* Small delay used by hardware drivers */
void io_wait(void);

#endif
