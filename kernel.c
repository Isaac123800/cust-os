// kernel.c
// Custos OS command shell

#include <stdbool.h>


char *video = (char *)0xB8000;

int cursor = 0;


bool equal(char *a, char *b);



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



void scroll()
{
    for(int row = 1; row < 25; row++)
    {
        for(int col = 0; col < 80; col++)
        {
            video[((row-1)*80+col)*2] =
            video[(row*80+col)*2];

            video[((row-1)*80+col)*2+1] =
            video[(row*80+col)*2+1];
        }
    }


    for(int col = 0; col < 80; col++)
    {
        video[(24*80+col)*2] = ' ';
        video[(24*80+col)*2+1] = 0x07;
    }


    cursor = 24*80;
}



void putchar(char c)
{
    if(c == '\n')
    {
        cursor = ((cursor/80)+1)*80;
    }
    else
    {
        video[cursor*2] = c;
        video[cursor*2+1] = 0x07;
        cursor++;
    }


    if(cursor >= 80*25)
    {
        scroll();
    }
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



void clear()
{
    for(int i = 0; i < 80*25; i++)
    {
        video[i*2] = ' ';
        video[i*2+1] = 0x07;
    }

    cursor = 0;
}



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



int find_command(char *name)
{
    for(int i = 0; i < command_count; i++)
    {
        if(equal(commands[i].name,name))
        {
            return i;
        }
    }

    return -1;
}



void enable(char *name)
{
    int id = find_command(name);


    if(id == -1)
    {
        print("\nCommand not found\n");
        return;
    }


    commands[id].enabled = true;

    print("\nCommand enabled\n");
}



void disable(char *name)
{
    int id = find_command(name);


    if(id == -1)
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



void echo(char *text)
{
    print("\n");
    print(text);
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



bool enabled(char *name)
{
    int id = find_command(name);


    if(id == -1)
        return false;


    return commands[id].enabled;
}



void run_command(char *input)
{

    if(equal(input,"credits"))
    {
        if(enabled("credits"))
            credits();
        else
            print("\nerror: Command not Found or not Enabled\n");
    }


    else if(equal(input,"cmdlist"))
    {
        cmdlist();
    }


    else if(equal(input,"clear"))
    {
        if(enabled("clear"))
            clear();
        else
            print("\nerror: Command not Found or not Enabled\n");
    }


    else if(input[0]=='e' &&
            input[1]=='c' &&
            input[2]=='h' &&
            input[3]=='o' &&
            input[4]==' ')
    {
        if(enabled("echo"))
            echo(input+5);
        else
            print("\nerror: Command not Found or not Enabled\n");
    }


    else if(input[0]=='e' &&
            input[1]=='n' &&
            input[2]=='a' &&
            input[3]=='b' &&
            input[4]=='l' &&
            input[5]=='e' &&
            input[6]==' ')
    {
        enable(input+7);
    }


    else if(input[0]=='d' &&
            input[1]=='i' &&
            input[2]=='s' &&
            input[3]=='a' &&
            input[4]=='b' &&
            input[5]=='l' &&
            input[6]=='e' &&
            input[7]==' ')
    {
        disable(input+8);
    }


    else
    {
        print("\nerror: Command not Found or not Enabled\n");
    }

}



char keyboard()
{
    unsigned char status;
    unsigned char key;


    char map[] =
    {
        0,27,
        '1','2','3','4','5','6','7','8','9','0',
        '-','=',8,9,
        'q','w','e','r','t','y','u','i','o','p',
        '[',']',13,0,
        'a','s','d','f','g','h','j','k','l',
        ';','\'','`',0,'\\',
        'z','x','c','v','b','n','m',
        ',','.','/',
        0,'*',0,' '
    };


    while(1)
    {
        __asm__ volatile("inb $0x64, %0" : "=a"(status));


        if(status & 1)
        {
            __asm__ volatile("inb $0x60, %0" : "=a"(key));


            if(key < 128)
                return map[key];
        }
    }
}



void kernel_main()
{
    clear();

    print("Welcome to Custos\n");
    print("Type commands:\n\n>");

    char input[100];

    int pos = 0;


    while(1)
    {
        char c = keyboard();


        if(c == 13)
        {
            input[pos] = '\0';

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

