#include "vga.h"

#define VGA_MEMORY ((volatile uint8_t *)0xA0000)

static bool graphics_mode = false;


/*
    Set VGA video mode.

    Mode 0x13 = 320x200, 256 colours.
    Mode 0x03 = 80x25 text mode.

    IMPORTANT:
    This function requires real-mode BIOS access.
    It cannot call INT 10h directly from your
    current 32-bit protected-mode kernel.
*/
void vga_set_mode(uint8_t mode)
{
    (void)mode;

    /*
        We will implement the actual mode switch
        when we connect the boot path.
    */
    graphics_mode = false;
}


void vga_put_pixel(
    uint16_t x,
    uint16_t y,
    uint8_t color
)
{
    if (!graphics_mode)
        return;

    if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
        return;

    VGA_MEMORY[y * VGA_WIDTH + x] = color;
}


void vga_fill_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint8_t color
)
{
    if (!graphics_mode)
        return;

    for (uint16_t row = 0; row < height; row++)
    {
        for (uint16_t column = 0; column < width; column++)
        {
            uint32_t px = (uint32_t)x + column;
            uint32_t py = (uint32_t)y + row;

            if (px >= VGA_WIDTH || py >= VGA_HEIGHT)
                continue;

            VGA_MEMORY[py * VGA_WIDTH + px] = color;
        }
    }
}


void vga_clear(uint8_t color)
{
    vga_fill_rect(
        0, 0,
        VGA_WIDTH, VGA_HEIGHT,
        color
    );
}
