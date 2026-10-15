#ifndef VGA_H
#define VGA_H

#include "../include/types.h"

/* VGA Mode 13h dimensions */
#define VGA_WIDTH  320
#define VGA_HEIGHT 200

/* Enter graphics mode and return to text mode */
void vga_set_mode(uint8_t mode);

/* Draw a pixel */
void vga_put_pixel(
    uint16_t x,
    uint16_t y,
    uint8_t color
);

/* Draw a filled rectangle */
void vga_fill_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint8_t color
);

/* Clear the screen */
void vga_clear(uint8_t color);

#endif
