#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

void scroll(void);

void putchar(char c);

void print(char *text);

void print_hex(uint8_t value);

void clear(void);

void backspace(void);

#endif

