#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

void scroll(void);

void putchar(char c);

void print(char *text);

void print_hex(uint8_t value);

void clear(void);

void backspace(void);

/* Set foreground and background colours. */
void console_set_color(
    uint8_t foreground,
    uint8_t background
);

#endif

