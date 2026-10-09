// CustOS main kernel
// This kernel is installed to the hard disk.
// It does not contain the installer or ISO code.

#include <stdbool.h>

#include "include/types.h"
#include "include/console.h"

#include "drivers/disk.h"
#include "fs/filesystem.h"


// ============================================================
// LANGUAGE SYSTEM
// ============================================================

static bool polish_language = false;

typedef struct
{
    const char *english;
    const char *polish;
} Translation;


// Call the original console printer without translation.
static void console_print_raw(char *text)
{
    print(text);
}


static bool same_message(
    const char *a,
    const char *b
)
{
    while(*a && *b)
    {
        if(*a != *b)
        {
            return false;
        }

        a++;
        b++;
    }

    return *a == *b;
}


static const Translation translations[] =
{
    // Shutdown and restart
    {
        "\nShutting down CustOS...\n",
        "\nZamykanie CustOS...\n"
    },
    {
        "\nRestarting CustOS...\n",
        "\nPonowne uruchamianie CustOS...\n"
    },

    // Credits and command list
    {
        "\nMade By:\n",
        "\nAutorzy:\n"
    },
    {
        "\nCommands:\n",
        "\nPolecenia:\n"
    },
    {
        " Protected\n",
        " Chronione\n"
    },
    {
        " Enabled\n",
        " Wlaczone\n"
    },
    {
        " Disabled\n",
        " Wylaczone\n"
    },

    // General command messages
    {
        "\nCommand not found\n",
        "\nNie znaleziono polecenia\n"
    },
    {
        "\nCommand enabled\n",
        "\nPolecenie wlaczone\n"
    },
    {
        "\nCannot disable protected command\n",
        "\nNie mozna wylaczyc chronionego polecenia\n"
    },
    {
        "\nCommand disabled\n",
        "\nPolecenie wylaczone\n"
    },

    // Colour command
    {
        "\nForeground colours (0-15):\n",
        "\nKolory tekstu (0-15):\n"
    },
    {
        "0 Black\n",
        "0 Czarny\n"
    },
    {
        "1 Blue\n",
        "1 Niebieski\n"
    },
    {
        "2 Green\n",
        "2 Zielony\n"
    },
    {
        "3 Cyan\n",
        "3 Turkusowy\n"
    },
    {
        "4 Red\n",
        "4 Czerwony\n"
    },
    {
        "5 Magenta\n",
        "5 Magenta\n"
    },
    {
        "6 Brown\n",
        "6 Brazowy\n"
    },
    {
        "7 Light grey\n",
        "7 Jasnoszary\n"
    },
    {
        "8 Dark grey\n",
        "8 Ciemnoszary\n"
    },
    {
        "9 Light blue\n",
        "9 Jasnoniebieski\n"
    },
    {
        "10 Light green\n",
        "10 Jasnozielony\n"
    },
    {
        "11 Light cyan\n",
        "11 Jasnoturkusowy\n"
    },
    {
        "12 Light red\n",
        "12 Jasnoczerwony\n"
    },
    {
        "13 Light magenta\n",
        "13 Jasna magenta\n"
    },
    {
        "14 Yellow\n",
        "14 Zolty\n"
    },
    {
        "15 White\n",
        "15 Bialy\n"
    },
    {
        "\nBackground colours: 0-7\n",
        "\nKolory tla: 0-7\n"
    },
    {
        "Usage: color <foreground> <background>\n",
        "Uzycie: color <tekst> <tlo>\n"
    },
    {
        "Example: color 15 0\n",
        "Przyklad: color 15 0\n"
    },
    {
        "\nUsage: color <foreground 0-15> <background 0-7>\n",
        "\nUzycie: color <tekst 0-15> <tlo 0-7>\n"
    },
    {
        "\nConsole colours changed and saved.\n",
        "\nKolory konsoli zmienione i zapisane.\n"
    },
    {
        "\nColours changed, but saving failed.\n",
        "\nKolory zmienione, ale zapis sie nie powiodl.\n"
    },

    // Filesystem messages
    {
        "\nFile created\n",
        "\nPlik utworzony\n"
    },
    {
        "\nCould not create file\n",
        "\nNie mozna utworzyc pliku\n"
    },
    {
        "\nFile not found\n",
        "\nNie znaleziono pliku\n"
    },
    {
        "\nEnter text:\n",
        "\nWpisz tekst:\n"
    },
    {
        "\nSaved\n",
        "\nZapisano\n"
    },
    {
        "\nWrite failed\n",
        "\nBlad zapisu\n"
    },
    {
        "\nDeleted\n",
        "\nUsunieto\n"
    },

    // Language command
    {
        "Language: English\n",
        "Jezyk: angielski\n"
    },
    {
        "Language: Polish\n",
        "Jezyk: polski\n"
    },
    {
        "Usage: lang en | lang pl\n",
        "Uzycie: lang en | lang pl\n"
    },
    {
        "Unknown language. Use en or pl.\n",
        "Nieznany jezyk. Uzyj en lub pl.\n"
    },
    {
        "\nLanguage changed and saved.\n",
        "\nJezyk zmieniony i zapisany.\n"
    },
    {
        "\nLanguage changed, but saving failed.\n",
        "\nJezyk zmieniony, ale zapis sie nie powiodl.\n"
    },

    // Fake Windows-style BSOD
    {
        "A problem has been detected in CustOS.\n",
        "W systemie CustOS wykryto problem.\n"
    },
    {
        "CustOS has been shut down to prevent damage\n",
        "CustOS zostal zatrzymany, aby zapobiec\n"
    },
    {
        "to your computer.\n\n",
        "uszkodzeniu komputera.\n\n"
    },
    {
        "WININIT_ERROR\n\n",
        "BLAD_WININIT\n\n"
    },
    {
        "If this is the first time you have seen this\n",
        "Jesli widzisz ten ekran po raz pierwszy,\n"
    },
    {
        "screen, restart CustOS. If this screen appears\n",
        "uruchom ponownie CustOS. Jesli ekran pojawi sie\n"
    },
    {
        "again, follow these steps:\n\n",
        "ponownie, wykonaj nastepujace kroki:\n\n"
    },
    {
        "Check your system configuration.\n",
        "Sprawdz konfiguracje systemu.\n"
    },
    {
        "If problems continue, restart your computer.\n\n",
        "Jesli problem nie zniknie, uruchom komputer\nponownie.\n\n"
    },
    {
        "Technical information:\n\n",
        "Informacje techniczne:\n\n"
    },
    {
        "Press any key to restart CustOS...",
        "Nacisnij dowolny klawisz, aby uruchomic CustOS ponownie..."
    }
};


// Translate known interface messages. Everything else
// is passed unchanged to the original console printer.
static void print_localized(char *text)
{
    if(polish_language)
    {
        for(unsigned int i = 0;
            i < sizeof(translations) / sizeof(translations[0]);
            i++)
        {
            if(same_message(text, translations[i].english))
            {
                console_print_raw(
                    (char *)translations[i].polish
                );

                return;
            }
        }
    }

    console_print_raw(text);
}


// Route this source file's interface messages through
// the translation function. Other source files are unaffected.
#define print(text) print_localized(text)


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
// PORT I/O HELPERS
// ============================================================

static uint8_t inb_from_port(uint16_t port)
{
    uint8_t value;

    __asm__ volatile(
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}


static void outb_to_port(
    uint16_t port,
    uint8_t value
)
{
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}


static void outw_to_port(
    uint16_t port,
    uint16_t value
)
{
    __asm__ volatile(
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}


// ============================================================
// SHUTDOWN
// ============================================================

static void shutdown_system(void)
{
    print("\nShutting down CustOS...\n");

    __asm__ volatile("cli");

    // Common QEMU/Bochs-compatible shutdown interfaces.
    outw_to_port(0x604, 0x2000);
    outw_to_port(0xB004, 0x2000);

    while(1)
    {
        __asm__ volatile("hlt");
    }
}


// ============================================================
// RESTART
// ============================================================

struct IDTPointer
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


static void restart_system(void)
{
    print("\nRestarting CustOS...\n");

    __asm__ volatile("cli");

    for(uint32_t i = 0; i < 100000; i++)
    {
        if((inb_from_port(0x64) & 0x02) == 0)
        {
            break;
        }
    }

    outb_to_port(0x64, 0xFE);

    // Fallback reset through an x86 triple fault.
    struct IDTPointer idtr = {0, 0};

    __asm__ volatile(
        "lidt %0\n\t"
        "int $3"
        :
        : "m"(idtr)
    );

    while(1)
    {
        __asm__ volatile("hlt");
    }
}


// ============================================================
// FAKE WINDOWS-STYLE BSOD EASTER EGG
// ============================================================

static void fake_bsod(void)
{
    // White text on a blue background.
    console_set_color(15, 1);
    clear();

    print("A problem has been detected in CustOS.\n");
    print("CustOS has been shut down to prevent damage\n");
    print("to your computer.\n\n");

    print("WININIT_ERROR\n\n");

    print("If this is the first time you have seen this\n");
    print("screen, restart CustOS. If this screen appears\n");
    print("again, follow these steps:\n\n");

    print("Check your system configuration.\n");
    print("If problems continue, restart your computer.\n\n");

    print("Technical information:\n\n");

    print("*** STOP: CUSTOS_NOT_WINDOWS\n\n");

    print("Press any key to restart CustOS...");

    keyboard();

    restart_system();
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
    {"color", true, false},
    {"shutdown", true, true},
    {"restart", true, true},
    {"wininit", true, false},
    {"lang", true, true}
};


int command_count = 16;


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
// SAVE AND LOAD LANGUAGE
// ============================================================

static bool save_language_setting(void)
{
    char value = polish_language ? 'p' : 'e';

    // Create the file the first time it is used.
    fs_create("LANG.CFG");

    return fs_write(
        "LANG.CFG",
        &value,
        1
    );
}


static void load_language_setting(void)
{
    // Static storage avoids a large local stack allocation.
    static char settings[FS_MAX_FILE_SIZE + 1];

    settings[0] = 0;
    settings[1] = 0;

    if(!fs_read("LANG.CFG", settings))
    {
        polish_language = false;
        return;
    }

    // This configuration file must contain exactly one byte.
    if(settings[1] != '\0')
    {
        polish_language = false;
        return;
    }

    if(settings[0] == 'p')
    {
        polish_language = true;
    }
    else
    {
        polish_language = false;
    }
}


// ============================================================
// SHOW CURRENT LANGUAGE
// ============================================================

static void show_language(void)
{
    if(polish_language)
    {
        print("Language: Polish\n");
    }
    else
    {
        print("Language: English\n");
    }

    print("Usage: lang en | lang pl\n");
}


// ============================================================
// CHANGE LANGUAGE
// ============================================================

static void set_language_command(char *args)
{
    // Remove leading spaces.
    while(*args == ' ')
    {
        args++;
    }

    // Remove trailing spaces.
    int length = 0;

    while(args[length])
    {
        length++;
    }

    while(length > 0 && args[length - 1] == ' ')
    {
        args[--length] = '\0';
    }

    if(equal(args, "en"))
    {
        polish_language = false;
    }
    else if(equal(args, "pl"))
    {
        polish_language = true;
    }
    else
    {
        print("Unknown language. Use en or pl.\n");
        return;
    }

    if(save_language_setting())
    {
        print("\nLanguage changed and saved.\n");
    }
    else
    {
        print("\nLanguage changed, but saving failed.\n");
    }
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
    int id = find_command(name);

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
        // Print command names literally; don't translate them.
        console_print_raw(commands[i].name);

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
    int id = find_command(name);

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

void disable_command(
    char *name
)
{
    int id = find_command(name);

    if(id < 0)
    {
        print("\nCommand not found\n");
        return;
    }

    if(commands[id].protected)
    {
        print("\nCannot disable protected command\n");
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

        // Check before multiplying to prevent overflow.
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
// SAVE COLOR SETTINGS
// ============================================================

static bool save_color_settings(
    uint8_t foreground,
    uint8_t background
)
{
    char hex[] = "0123456789ABCDEF";

    char settings[10];

    settings[0] = 'C';
    settings[1] = 'S';
    settings[2] = 'T';
    settings[3] = 'C';
    settings[4] = 'O';
    settings[5] = 'L';
    settings[6] = 'O';
    settings[7] = 'R';

    settings[8] = hex[foreground];
    settings[9] = (char)('0' + background);

    fs_create("COLOR.CFG");

    return fs_write(
        "COLOR.CFG",
        settings,
        sizeof(settings)
    );
}


// ============================================================
// LOAD COLOR SETTINGS
// ============================================================

static void load_color_settings(void)
{
    char settings[FS_MAX_FILE_SIZE + 1];

    settings[0] = 0;
    settings[1] = 0;
    settings[2] = 0;
    settings[3] = 0;
    settings[4] = 0;
    settings[5] = 0;
    settings[6] = 0;
    settings[7] = 0;
    settings[8] = 0;
    settings[9] = 0;
    settings[10] = 0;

    if(!fs_read("COLOR.CFG", settings))
    {
        return;
    }

    if(settings[0] != 'C' ||
       settings[1] != 'S' ||
       settings[2] != 'T' ||
       settings[3] != 'C' ||
       settings[4] != 'O' ||
       settings[5] != 'L' ||
       settings[6] != 'O' ||
       settings[7] != 'R')
    {
        return;
    }

    if(settings[10] != '\0')
    {
        return;
    }

    char hex[] = "0123456789ABCDEF";

    int foreground = -1;

    for(int i = 0; i < 16; i++)
    {
        if(settings[8] == hex[i])
        {
            foreground = i;
            break;
        }
    }

    if(settings[9] < '0' ||
       settings[9] > '7')
    {
        return;
    }

    uint8_t background =
        (uint8_t)(settings[9] - '0');

    if(foreground < 0 || foreground > 15)
    {
        return;
    }

    console_set_color(
        (uint8_t)foreground,
        background
    );
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

    if(save_color_settings(
            (uint8_t)foreground,
            (uint8_t)background))
    {
        print("\nConsole colours changed and saved.\n");
    }
    else
    {
        print("\nColours changed, but saving failed.\n");
    }
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

        // File contents are data, not interface messages.
        console_print_raw(buffer);
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
        char c = keyboard();

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
            if(pos < FS_MAX_FILE_SIZE - 1)
            {
                buffer[pos++] = c;

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

    // Echo exactly what the user entered.
    console_print_raw(text);
}


// ============================================================
// COMMAND PARSER
// ============================================================

void run_command(
    char *input
)
{
    if(equal(input, "credits"))
    {
        if(enabled("credits"))
        {
            credits();
        }
    }

    else if(equal(input, "cmdlist"))
    {
        cmdlist();
    }

    else if(equal(input, "clear"))
    {
        clear();
    }

    else if(starts(input, "echo "))
    {
        if(enabled("echo"))
        {
            echo(input + 5);
        }
    }

    else if(starts(input, "enable "))
    {
        enable_command(input + 7);
    }

    else if(starts(input, "disable "))
    {
        disable_command(input + 8);
    }

    else if(starts(input, "touch "))
    {
        if(enabled("touch"))
        {
            touch(input + 6);
        }
    }

    else if(starts(input, "write "))
    {
        if(enabled("write"))
        {
            write_file(input + 6);
        }
    }

    else if(starts(input, "cat "))
    {
        if(enabled("cat"))
        {
            cat(input + 4);
        }
    }

    else if(equal(input, "ls"))
    {
        if(enabled("ls"))
        {
            fs_list();
        }
    }

    else if(starts(input, "rm "))
    {
        if(enabled("rm"))
        {
            remove_file(input + 3);
        }
    }

    else if(starts(input, "color "))
    {
        if(enabled("color"))
        {
            set_color_command(input + 6);
        }
    }

    else if(equal(input, "color"))
    {
        if(enabled("color"))
        {
            show_colors();
        }
    }

    else if(starts(input, "lang "))
    {
        if(enabled("lang"))
        {
            set_language_command(input + 5);
        }
    }

    else if(equal(input, "lang"))
    {
        if(enabled("lang"))
        {
            show_language();
        }
    }

    else if(equal(input, "shutdown"))
    {
        if(enabled("shutdown"))
        {
            shutdown_system();
        }
    }

    else if(equal(input, "restart"))
    {
        if(enabled("restart"))
        {
            restart_system();
        }
    }

    else if(equal(input, "wininit"))
    {
        if(enabled("wininit"))
        {
            fake_bsod();
        }
    }

    else
    {
        print("\nCommand not found\n");
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
            char c = keyboard();

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

            if(c >= 32 && c <= 126)
            {
                if(pos < 127)
                {
                    input[pos++] = c;

                    putchar(c);
                }
            }
        }
    }
}


// ============================================================
// KERNEL ENTRY
// ============================================================

__attribute__((section(".text.kernel_main")))
void kernel_main()
{
    clear();

    print("CustOS kernel booted!\n");
    print("====================\n\n");

    print("Initializing disk...\n");

    disk_init();

    print("Disk ready!\n");
    print("Mounting filesystem...\n");

    if(!fs_mount())
    {
        // Language settings can't be loaded if mounting fails.
        print("Filesystem mount failed!\n");

        while(1)
        {
        }
    }

    // Load language and colours after mounting the filesystem.
    load_language_setting();
    load_color_settings();

    print("Filesystem mounted!\n");
    print("Filesystem ready!\n");

    shell();
}

