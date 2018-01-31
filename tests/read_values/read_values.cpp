#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "GPIOClass.h"

using namespace std;

int main (void)
{

    string inputstate;
    GPIOClass* gpio18 = new GPIOClass("18"); //create new GPIO object to be attached to  GPIO18

    if (gpio18->export_gpio() == - 1) {return -1;} //export GPIO18
    cout << " GPIO pins exported" << endl;
    
    if (gpio18->setdir_gpio("in") == -1) {return -1;} //GPIO18 set to input
    cout << " Set GPIO pin directions" << endl;

    while(1)
    {
        usleep(500000);  // wait for 0.5 seconds
        gpio18->getval_gpio(inputstate); //read state of GPIO18 input pin
        cout << "Current input pin state is " << inputstate  <<endl;
        
        if(inputstate == "0") // if input pin is at state "0" i.e. button pressed
        {
            cout << "input pin state is \"Pressed \".n Will check input pin state again in 20ms "<<endl;
            usleep(20000);
            cout << "Checking again ....." << endl;
            gpio18->getval_gpio(inputstate); // checking again to ensure that state "0" is due to button press and not noise
            
            if(inputstate == "0")
            {

                cout << " Waiting until pin is unpressed....." << endl;
                while (inputstate == "0")
                {
                    gpio18->getval_gpio(inputstate);
                };
                cout << "pin is unpressed" << endl;

            }
            else
                cout << "input pin state is definitely \"UnPressed\". That was just noise." <<endl;

        }

    }
    cout << "Exiting....." << endl;
    return 0;
}