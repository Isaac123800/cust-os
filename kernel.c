// kernel.c
// Custos command system

#include <stdbool.h>

char *video_memory = (char *)0xB8000;
int cursor = 0;

bool equal(char *a, char *b);


void print(char *text)
{
    int i = 0;

    while (text[i] != '\0')
    {
        if (text[i] == '\n')
        {
            cursor = ((cursor / 160) + 1) * 160;
        }
        else
        {
            video_memory[cursor] = text[i];
            video_memory[cursor + 1] = 0x07;
            cursor += 2;
        }

        i++;
    }
}


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
    {"help", true, false},
    {"echo", true, false},
    {"clear", true, false}
};


int command_count = 7;


void credits()
{
    print("Made By:\n");
    print("Isaac Polomski\n");
    print("Roshan Inbasekar\n");
}


void cmdlist()
{
    print("Commands:\n");

    for (int i = 0; i < command_count; i++)
    {
        print(commands[i].name);

        if (commands[i].protected)
        {
            print(" Protected\n");
        }
        else if (commands[i].enabled)
        {
            print(" Enabled\n");
        }
        else
        {
            print(" Disabled\n");
        }
    }
}


void enable(char *name)
{
    for (int i = 0; i < command_count; i++)
    {
        if (equal(commands[i].name, name))
        {
            commands[i].enabled = true;
            print("Command enabled\n");
            return;
        }
    }

    print("Command not found\n");
}


void disable(char *name)
{
    for (int i = 0; i < command_count; i++)
    {
        if (equal(commands[i].name, name))
        {
            if (commands[i].protected)
            {
                print("Cannot disable protected command\n");
                return;
            }

            commands[i].enabled = false;
            print("Command disabled\n");
            return;
        }
    }

    print("Command not found\n");
}


bool equal(char *a, char *b)
{
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return false;
        }

        i++;
    }

    return a[i] == b[i];
}


void kernel_main()
{
    print("Welcome to Custos\n\n");

    credits();

    print("\nType commands:\n");

    while (1)
    {

    }
}
