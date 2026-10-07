// CustOS main kernel
// This kernel is copied to the installed disk.

#include <stdbool.h>

#include "include/types.h"

#include "drivers/disk.h"

#include "fs/filesystem.h"


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
// BACKSPACE
// ============================================================

void backspace()
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


// ============================================================
// KEYBOARD
// ============================================================

char keyboard()
{
    unsigned char status;
    unsigned char key;

    char map[] =
    {
        0, 27,

        '1','2','3','4','5',
        '6','7','8','9','0',

        '-','=',
        8,9,

        'q','w','e','r','t',
        'y','u','i','o','p',

        '[',']',
        13,0,

        'a','s','d','f','g',
        'h','j','k','l',

        ';','\'','`',
        0,'\\',

        'z','x','c','v','b',
        'n','m',

        ',','.','/',

        0,'*',0,' '
    };

    while(1)
    {
        __asm__ volatile(
            "inb $0x64, %0"
            : "=a"(status)
        );

        if(status & 1)
        {
            __asm__ volatile(
                "inb $0x60, %0"
                : "=a"(key)
            );

            if(key < sizeof(map))
            {
                return map[key];
            }
        }
    }
}


// ============================================================
// COMMAND SYSTEM
// ============================================================

typedef struct
{
    char name[20];

    bool enabled;

    bool protected;

} Command;


Command commands[] =
{
    {"credits", true, false},
    {"cmdlist", true, true},
    {"enable", true, true},
    {"disable", true, true},
    {"echo", true, false},
    {"clear", true, false},
    {"touch", true, false},
    {"write", true, false},
    {"cat", true, false},
    {"ls", true, false},
    {"rm", true, false},
    {"color", true, false}
};


int command_count = 12;


// ============================================================
// STRING EQUAL
// ============================================================

bool equal(
    char *a,
    char *b
)
{
    int i = 0;

    while(a[i] && b[i])
    {
        if(a[i] != b[i])
        {
            return false;
        }

        i++;
    }

    return a[i] == b[i];
}


// ============================================================
// STRING STARTS
// ============================================================

bool starts(
    char *a,
    char *b
)
{
    int i = 0;

    while(b[i])
    {
        if(a[i] != b[i])
        {
            return false;
        }

        i++;
    }

    return true;
}


// ============================================================
// FIND COMMAND
// ============================================================

int find_command(char *name)
{
    for(int i = 0;
        i < command_count;
        i++)
    {
        if(equal(
            commands[i].name,
            name))
        {
            return i;
        }
    }

    return -1;
}


// ============================================================
// COMMAND ENABLED
// ============================================================

bool enabled(char *name)
{
    int id =
        find_command(name);

    if(id < 0)
    {
        return false;
    }

    return commands[id].enabled;
}


// ============================================================
// CREDITS
// ============================================================

void credits()
{
    print("\nMade By:\n");

    print("Isaac Polomski\n");
    print("Roshan Inbasekar\n");
    print("Kirthis Kirubaventhan\n");
}


// ============================================================
// COMMAND LIST
// ============================================================

void cmdlist()
{
    print("\nCommands:\n");

    for(int i = 0;
        i < command_count;
        i++)
    {
        print(commands[i].name);

        if(commands[i].protected)
        {
            print(" Protected\n");
        }
        else if(commands[i].enabled)
        {
            print(" Enabled\n");
        }
        else
        {
            print(" Disabled\n");
        }
    }
}


// ============================================================
// ENABLE COMMAND
// ============================================================

void enable_command(char *name)
{
    int id =
        find_command(name);

    if(id < 0)
    {
        print("\nCommand not found\n");
        return;
    }

    commands[id].enabled = true;

    print("\nCommand enabled\n");
}


// ============================================================
// DISABLE COMMAND
// ============================================================

void disable_command(char *name)
{
    int id =
        find_command(name);

    if(id < 0)
    {
        print("\nCommand not found\n");
        return;
    }

    if(commands[id].protected)
    {
        print(
            "\nCannot disable protected command\n"
        );

        return;
    }

    commands[id].enabled = false;

    print("\nCommand disabled\n");
}


// ============================================================
// SHOW COLORS
// ============================================================

void show_colors()
{
    print("\nColors:\n");

    print("0 Black\n");
    print("1 Red\n");
    print("2 Green\n");
    print("3 Yellow\n");
    print("4 Blue\n");
    print("5 Magenta\n");
    print("6 Cyan\n");
    print("7 White\n");
}


// ============================================================
// FILESYSTEM COMMANDS
// ============================================================

void touch(char *name)
{
    if(fs_create(name))
    {
        print("\nFile created\n");
    }
    else
    {
        print("\nCould not create file\n");
    }
}


// ============================================================
// CAT
// ============================================================

void cat(char *name)
{
    char buffer[
        FS_MAX_FILE_SIZE + 1
    ];

    if(fs_read(
        name,
        buffer))
    {
        print("\n");
        print(buffer);
    }
    else
    {
        print("\nFile not found\n");
    }
}


// ============================================================
// WRITE FILE
// ============================================================

void write_file(char *name)
{
    char buffer[
        FS_MAX_FILE_SIZE
    ];

    print("\nEnter text:\n");

    int pos = 0;

    while(1)
    {
        char c =
            keyboard();

        if(c == 13)
        {
            break;
        }

        if(c == 8)
        {
            if(pos > 0)
            {
                pos--;

                backspace();
            }
        }
        else
        {
            if(pos <
               FS_MAX_FILE_SIZE - 1)
            {
                buffer[pos++] =
                    c;

                putchar(c);
            }
        }
    }

    buffer[pos] = 0;

    if(fs_write(
        name,
        buffer,
        pos))
    {
        print("\nSaved\n");
    }
    else
    {
        print("\nWrite failed\n");
    }
}


// ============================================================
// REMOVE FILE
// ============================================================

void remove_file(char *name)
{
    if(fs_delete(name))
    {
        print("\nDeleted\n");
    }
    else
    {
        print("\nFile not found\n");
    }
}


// ============================================================
// ECHO
// ============================================================

void echo(char *text)
{
    print("\n");
    print(text);
}


// ============================================================
// COMMAND PARSER
// ============================================================

void run_command(char *input)
{
    if(equal(
        input,
        "credits"))
    {
        if(enabled("credits"))
        {
            credits();
        }
    }

    else if(equal(
        input,
        "cmdlist"))
    {
        cmdlist();
    }

    else if(equal(
        input,
        "clear"))
    {
        clear();
    }

    else if(starts(
        input,
        "echo "))
    {
        if(enabled("echo"))
        {
            echo(input + 5);
        }
    }

    else if(starts(
        input,
        "enable "))
    {
        enable_command(
            input + 7
        );
    }

    else if(starts(
        input,
        "disable "))
    {
        disable_command(
            input + 8
        );
    }

    else if(starts(
        input,
        "touch "))
    {
        if(enabled("touch"))
        {
            touch(input + 6);
        }
    }

    else if(starts(
        input,
        "write "))
    {
        if(enabled("write"))
        {
            write_file(input + 6);
        }
    }

    else if(starts(
        input,
        "cat "))
    {
        if(enabled("cat"))
        {
            cat(input + 4);
        }
    }

    else if(equal(
        input,
        "ls"))
    {
        if(enabled("ls"))
        {
            fs_list();
        }
    }

    else if(starts(
        input,
        "rm "))
    {
        if(enabled("rm"))
        {
            remove_file(
                input + 3
            );
        }
    }

    else if(equal(
        input,
        "color"))
    {
        show_colors();
    }

    else
    {
        print(
            "\nCommand not found\n"
        );
    }
}


// ============================================================
// SHELL
// ============================================================

void shell()
{
    char input[128];

    while(1)
    {
        print("\nCustOS> ");

        int pos = 0;

        while(1)
        {
            char c =
                keyboard();

            if(c == 13)
            {
                input[pos] = 0;

                print("\n");

                run_command(input);

                break;
            }

            if(c == 8)
            {
                if(pos > 0)
                {
                    pos--;

                    backspace();
                }

                continue;
            }

            if(c >= 32 &&
               c <= 126)
            {
                if(pos < 127)
                {
                    input[pos++] =
                        c;

                    putchar(c);
                }
            }
        }
    }
}


// ============================================================
// MAIN KERNEL ENTRY
// ============================================================

void kernel_main()
{
    clear();

    print(
        "CustOS kernel booted!\n"
    );

    print(
        "====================\n\n"
    );

    print(
        "Initializing disk...\n"
    );

    disk_init();

    print(
        "Disk ready!\n"
    );

    print(
        "Mounting filesystem...\n"
    );

    if(!fs_mount())
    {
        print(
            "Filesystem mount failed!\n"
        );

        while(1)
        {
        }
    }

    print(
        "Filesystem mounted!\n"
    );

    print(
        "Filesystem ready!\n"
    );

    shell();
}


