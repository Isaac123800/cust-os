#include "installer.h"

#include "install.h"


extern void clear(void);
extern void print(char *text);



static void delay()
{
    for(volatile int i = 0; i < 5000000; i++)
    {

    }
}



void installer_start(void)
{

    clear();


    print("Preparing for first CustOS boot...\n");


    delay();



    clear();


    print("CustOS Installer\n");
    print("================\n\n");


    print("Starting installation...\n");


    install_system();


    print("\nYou can now restart your computer.\n");
}
