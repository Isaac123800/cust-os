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


    print("Preparing CustOS Installer...\n");


    delay();



    clear();



    print("CustOS Installer\n");
    print("================\n\n");



    print("Starting installation...\n");


    print("Calling install_system...\n");



    bool result = install_system();



    print("Returned from install_system...\n");



    if(result)
    {
        print("\nInstallation successful!\n");

        print("Restart the machine to boot CustOS.\n");
    }

    else
    {
        print("\nInstallation failed!\n");
    }



    while(1)
    {

    }
}
