#include "../include/console.h"


// ============================================================
// VIDEO
// ============================================================

#define VIDEO_MEMORY 0xB8000

static char *video =
    (char *)VIDEO_MEMORY;

static int cursor = 0;

static int text_color = 7;

static int bg_color = 0;


// ============================================================
// SCROLL
// ============================================================

void scroll(void)
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
        video[(24 * 80 + x) * 2] =
            ' ';

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
        video[cursor * 2] =
            c;

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

void clear(void)
{
    for(int i = 0; i < 80 * 25; i++)
    {
        video[i * 2] =
            ' ';

        video[i * 2 + 1] =
            (bg_color << 4) | text_color;
    }

    cursor = 0;
}


// ============================================================
// BACKSPACE
// ============================================================

void backspace(void)
{
    if(cursor > 0)
    {
        cursor--;

        video[cursor * 2] =
            ' ';

        video[cursor * 2 + 1] =
            (bg_color << 4) | text_color;
    }
}
