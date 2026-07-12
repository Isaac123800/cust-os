// kernel.c
// Custos command shell

#include <stdbool.h>

char *video = (char *)0xB8000;
int cursor = 0;

bool equal(char *a, char *b);


void putchar(char c)
{
    if(c == '\n')
    {
        cursor = ((cursor / 80) + 1) * 80;
        return;
    }

    video[cursor * 2] = c;
    video[cursor * 2 + 1] = 0x07;
    cursor++;
}


void print(char *text)
{
    int i = 0;

    while(text[i])
    {
        putchar(text[i]);
        i++;
    }
}


char get_keyboard()
{
    unsigned char status;
    unsigned char key;

    while(1)
    {
        status = 0;
        __asm__ volatile ("inb $0x64, %0" : "=a"(status));

        if(status & 1)
        {
            __asm__ volatile ("inb $0x60, %0" : "=a"(key));

            if(key < 128)
            {
                char keys[] =
                {
                    0,27,'1','2','3','4','5','6','7','8','9','0',
                    '-','=',8,9,'q','w','e','r','t','y','u','i',
                    'o','p','[',']',13,0,'a','s','d','f','g','h',
                    'j','k','l',';',39,'`',0,'\\','z','x','c','v',
                    'b','n','m',',','.','/',0,'*',0,' '
                };

                return keys[key];
            }
        }
    }
}


typedef struct
{
    char name[20];
    bool enabled;
    bool protected;
}
Command;


Command commands[] =
{
    {"credits", true, false},
    {"cmdlist", true, true},
    {"enable", true, true},
    {"disable", true, true},
    {"echo", true, false},
    {"clear", true, false}
};


int command_count = 6;


void credits()
{
    print("\nMade By:\n");
    print("Isaac Polomski\n");
    print("Roshan Inbasekar\n");
}


void cmdlist()
{
    print("\nCommands:\n");

    for(int i = 0; i < command_count; i++)
    {
        print(commands[i].name);

        if(commands[i].protected)
            print(" Protected\n");
        else if(commands[i].enabled)
            print(" Enabled\n");
        else
            print(" Disabled\n");
    }
}


void disable(char *name)
{
    for(int i = 0; i < command_count; i++)
    {
        if(equal(commands[i].name,name))
        {
            if(commands[i].protected)
            {
                print("\nCannot disable protected command\n");
                return;
            }

            commands[i].enabled = false;
            print("\nCommand disabled\n");
            return;
        }
    }

    print("\nCommand not found\n");
}


void enable(char *name)
{
    for(int i = 0; i < command_count; i++)
    {
        if(equal(commands[i].name,name))
        {
            commands[i].enabled = true;
            print("\nCommand enabled\n");
            return;
        }
    }

    print("\nCommand not found\n");
}


bool equal(char *a, char *b)
{
    int i = 0;

    while(a[i] && b[i])
    {
        if(a[i] != b[i])
            return false;

        i++;
    }

    return a[i] == b[i];
}


void run_command(char *command)
{
    if(equal(command,"credits"))
    {
        credits();
    }
    else if(equal(command,"cmdlist"))
    {
        cmdlist();
    }
    else
    {
        print("\nerror: Command not Found or not Enabled\n");
    }
}


void kernel_main()
{
    print("Welcome to Custos\n");
    print("Type commands:\n\n>");

    char input[50];
    int pos = 0;

    while(1)
    {
        char c = get_keyboard();

        if(c == 13)
        {
            input[pos] = 0;

            run_command(input);

            pos = 0;

            print("\n>");
        }
        else
        {
            input[pos++] = c;
            putchar(c);
        }
    }
}
