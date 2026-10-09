// CustOS main kernel
// This kernel is installed to the hard disk.
// It does not contain the installer or ISO code.

#include <stdbool.h>

#include "include/types.h"
#include "include/console.h"

#include "drivers/disk.h"
#include "fs/filesystem.h"


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

int find_command(
    char *name
)
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

bool enabled(
    char *name
)
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

void enable_command(
    char *name
)
{
    int id =
        find_command(name);

    if(id < 0)
    {
        print("\nCommand not found\n");
        return;
    }

    commands[id].enabled =
        true;

    print("\nCommand enabled\n");
}


// ============================================================
// DISABLE COMMAND
// ============================================================

void disable_command(
    char *name
)
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

    commands[id].enabled =
        false;

    print("\nCommand disabled\n");
}


// ============================================================
// SHOW COLORS
// ============================================================

void show_colors()
{
    print("\nForeground colours (0-15):\n");

    print("0 Black\n");
    print("1 Blue\n");
    print("2 Green\n");
    print("3 Cyan\n");
    print("4 Red\n");
    print("5 Magenta\n");
    print("6 Brown\n");
    print("7 Light grey\n");
    print("8 Dark grey\n");
    print("9 Light blue\n");
    print("10 Light green\n");
    print("11 Light cyan\n");
    print("12 Light red\n");
    print("13 Light magenta\n");
    print("14 Yellow\n");
    print("15 White\n");

    print("\nBackground colours: 0-7\n");

    print("Usage: color <foreground> <background>\n");

    print("Example: color 15 0\n");
}


// ============================================================
// PARSE A COLOR NUMBER
// ============================================================

static bool parse_color_number(
    char **cursor,
    uint32_t maximum,
    uint32_t *result
)
{
    while(**cursor == ' ')
    {
        (*cursor)++;
    }

    if(**cursor < '0' ||
       **cursor > '9')
    {
        return false;
    }

    uint32_t value = 0;

    while(**cursor >= '0' &&
          **cursor <= '9')
    {
        uint32_t digit =
            (uint32_t)(**cursor - '0');

        /*
            Check the limit before multiplying.
            This also prevents long numbers from
            overflowing the integer.
        */

        if(digit > maximum ||
           value > (maximum - digit) / 10)
        {
            return false;
        }

        value = value * 10 + digit;

        (*cursor)++;
    }

    *result = value;

    return true;
}


// ============================================================
// SET CONSOLE COLOURS
// ============================================================

static void set_color_command(
    char *args
)
{
    uint32_t foreground;
    uint32_t background;

    if(!parse_color_number(
            &args, 15, &foreground) ||
       !parse_color_number(
            &args, 7, &background))
    {
        print(
            "\nUsage: color <foreground 0-15> <background 0-7>\n"
        );

        return;
    }

    while(*args == ' ')
    {
        args++;
    }

    if(*args != '\0')
    {
        print(
            "\nUsage: color <foreground 0-15> <background 0-7>\n"
        );

        return;
    }

    console_set_color(
        (uint8_t)foreground,
        (uint8_t)background
    );

    print("\nConsole colours changed.\n");
}


// ============================================================
// FILESYSTEM COMMANDS
// ============================================================

void touch(
    char *name
)
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

void cat(
    char *name
)
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

void write_file(
    char *name
)
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

void remove_file(
    char *name
)
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

void echo(
    char *text
)
{
    print("\n");
    print(text);
}


// ============================================================
// COMMAND PARSER
// ============================================================

void run_command(
    char *input
)
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
            write_file(
                input + 6
            );
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

    else if(starts(
        input,
        "color "))
    {
        if(enabled("color"))
        {
            set_color_command(
                input + 6
            );
        }
    }

    else if(equal(
        input,
        "color"))
    {
        if(enabled("color"))
        {
            show_colors();
        }
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

                run_command(
                    input
                );

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
// KERNEL ENTRY
// ============================================================
//
// This is the actual installed CustOS kernel.
//
// The linker places this function at 0x100000.
// The BIOS bootloader loads kernel.bin there and
// jumps directly to it.
// ============================================================

__attribute__((section(".text.kernel_main")))
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

