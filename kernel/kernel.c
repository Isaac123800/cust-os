// Custos OS kernel.c
// Disk filesystem version

#include <stdbool.h>

#include "include/types.h"

#include "drivers/disk.h"
#include "fs/filesystem.h"
#include "installer/installer.h"


#define VIDEO_MEMORY 0xB8000


char *video = (char *)VIDEO_MEMORY;


int cursor = 0;


int text_color = 7;
int bg_color = 0;



void scroll()
{
    for(int y = 1; y < 25; y++)
    {
        for(int x = 0; x < 80; x++)
        {
            video[((y-1)*80+x)*2] =
            video[(y*80+x)*2];


            video[((y-1)*80+x)*2+1] =
            video[(y*80+x)*2+1];
        }
    }


    for(int x = 0; x < 80; x++)
    {
        video[(24*80+x)*2] = ' ';


        video[(24*80+x)*2+1] =
        (bg_color << 4) | text_color;
    }


    cursor = 24*80;
}




void putchar(char c)
{

    if(c == '\n')
    {
        cursor =
        ((cursor / 80)+1)*80;
    }

    else
    {
        video[cursor*2] = c;


        video[cursor*2+1] =
        (bg_color << 4) | text_color;


        cursor++;
    }


    if(cursor >= 80*25)
        scroll();
}





void print(char *text)
{
    while(*text)
    {
        putchar(*text);

        text++;
    }
}





void clear()
{
    for(int i = 0; i < 80*25; i++)
    {
        video[i*2] = ' ';


        video[i*2+1] =
        (bg_color << 4) | text_color;
    }


    cursor = 0;
}





void backspace()
{
    if(cursor > 0)
    {
        cursor--;


        video[cursor*2] = ' ';


        video[cursor*2+1] =
        (bg_color << 4) | text_color;
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


            if(key < 128)
                return map[key];
        }
    }
}
/*
    COMMAND SYSTEM
*/


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




bool starts(char *a, char *b)
{
    int i = 0;


    while(b[i])
    {
        if(a[i] != b[i])
            return false;

        i++;
    }


    return true;
}





int find_command(char *name)
{
    for(int i = 0;
        i < command_count;
        i++)
    {
        if(equal(commands[i].name,name))
            return i;
    }


    return -1;
}





bool enabled(char *name)
{
    int id =
        find_command(name);


    if(id < 0)
        return false;


    return commands[id].enabled;
}





void credits()
{
    print("\nMade By:\n");

    print("Isaac Polomski\n");

    print("Roshan Inbasekar\n");

    print("Kirthis Kirubaventhan\n");
}





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
        print("\nCannot disable protected command\n");

        return;
    }



    commands[id].enabled = false;


    print("\nCommand disabled\n");
}





int get_number(char *s)
{
    int n = 0;


    while(*s >= '0' && *s <= '9')
    {
        n =
        n * 10 + (*s - '0');

        s++;
    }


    return n;
}





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
    print("8 Gray\n");
    print("9 Light Red\n");
    print("10 Light Green\n");
    print("11 Light Yellow\n");
    print("12 Light Blue\n");
    print("13 Light Magenta\n");
    print("14 Light Cyan\n");
    print("15 Bright White\n");
}





void set_background(char *n)
{
    bg_color =
        get_number(n);


    clear();


    print("Background changed\n");
}





void set_text(char *n)
{
    text_color =
        get_number(n);


    print("Text color changed\n");
}
/*
    FILESYSTEM COMMANDS
*/


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





void cat(char *name)
{
    char buffer[FS_MAX_FILE_SIZE + 1];


    if(fs_read(name, buffer))
    {
        print("\n");

        print(buffer);
    }

    else
    {
        print("\nFile not found\n");
    }
}





void write_file(char *name)
{
    char buffer[FS_MAX_FILE_SIZE];


    print("\nEnter text:\n");



    int pos = 0;



    while(1)
    {
        char c =
            keyboard();



        if(c == 13)
            break;



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
            if(pos < FS_MAX_FILE_SIZE-1)
            {
                buffer[pos++] = c;

                putchar(c);
            }
        }
    }



    buffer[pos] = 0;



    if(fs_write(name,buffer,pos))
    {
        print("\nSaved\n");
    }

    else
    {
        print("\nWrite failed\n");
    }
}





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
/*
    ECHO COMMAND
*/

void echo(char *text)
{
    print("\n");

    print(text);
}





/*
    COMMAND PARSER
*/

void run_command(char *input)
{

    if(equal(input,"credits"))
    {
        if(enabled("credits"))
            credits();
    }



    else if(equal(input,"cmdlist"))
    {
        cmdlist();
    }



    else if(equal(input,"clear"))
    {
        clear();
    }



    else if(starts(input,"echo "))
    {
        if(enabled("echo"))
            echo(input + 5);
    }



    else if(starts(input,"enable "))
    {
        enable_command(input + 7);
    }



    else if(starts(input,"disable "))
    {
        disable_command(input + 8);
    }



    else if(equal(input,"color"))
    {
        show_colors();
    }



    else if(starts(input,"color bg "))
    {
        set_background(input + 9);
    }



    else if(starts(input,"color txt "))
    {
        set_text(input + 10);
    }



    /*
        FILESYSTEM COMMANDS
    */


    else if(starts(input,"touch "))
    {
        if(enabled("touch"))
            touch(input + 6);
    }



    else if(starts(input,"write "))
    {
        if(enabled("write"))
            write_file(input + 6);
    }



    else if(starts(input,"cat "))
    {
        if(enabled("cat"))
            cat(input + 4);
    }



    else if(equal(input,"ls"))
    {
        if(enabled("ls"))
            fs_list();
    }



    else if(starts(input,"rm "))
    {
        if(enabled("rm"))
            remove_file(input + 3);
    }



    else
    {
        print("\nerror: Command not Found or not Enabled\n");
    }
}
/*
    KERNEL ENTRY
*/


void kernel_main()
{

    clear();



    /*
        Initialize hardware
    */

    print("1\n");

    disk_init();

    print("2\n");

    /*
        Load filesystem

        If no filesystem exists,
        it will format a new one.
    */

    fs_init();

    print("3\n");


    print("Welcome to Custos\n");

    print("Filesystem loaded\n");

    print("Type commands:\n\n>");





    char input[100];


    int pos = 0;





    while(1)
    {

        char c =
            keyboard();




        /*
            Enter key
        */

        if(c == 13)
        {

            input[pos] = 0;



            run_command(input);



            pos = 0;



            print("\n>");
        }





        /*
            Backspace
        */

        else if(c == 8)
        {

            if(pos > 0)
            {
                pos--;


                input[pos] = 0;


                backspace();
            }
        }





        /*
            Normal character
        */

        else if(c)
        {

            if(pos < 99)
            {
                input[pos++] = c;


                putchar(c);
            }
        }
    }
}
