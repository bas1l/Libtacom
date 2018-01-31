#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "GPIOClass.h"

#include <sys/mman.h>
#include "ad5383.h"
#include <queue>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <random>
#include <sys/ioctl.h>

using namespace std;

int main (void)
{

    string inputstate;
    GPIOClass* gpio18 = new GPIOClass("18"); //create new GPIO object to be attached to  GPIO18

    if (gpio18->export_gpio() == - 1) {return -1;} //export GPIO18
    cout << " GPIO pins exported" << endl;
    
    if (gpio18->setdir_gpio("in") == -1) {return -1;} //GPIO18 set to input
    cout << " Set GPIO pin directions" << endl;
    
    std::cout << "Neutral::Begin." << std::endl;

    struct timespec t;
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    /**************** C'EST AVEC CETTE OPTION QUE TOUT BUG *********************
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
            perror("sched_setscheduler failed");
            exit(-1);
    }
    ***************************************************************************/
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
    }

    
    AD5383 ad;
    ad.spi_open();
    ad.configure();
    std::vector<std::vector<uint16_t> > values(AD5383::num_channels);
    
    for(int j = 0; j < AD5383::num_channels; ++j)
    {
        values[j].push_back(2048);
    }
    
    long ms = 900;
    
    
    while(1)
    {
        usleep(50000);  // wait for 0.5 seconds
        gpio18->getval_gpio(inputstate); //read state of GPIO18 input pin
        cout << "Current input pin state is " << inputstate  <<endl;
        while (inputstate == "0")
        {
            gpio18->getval_gpio(inputstate);
        };
        std::cout << "Power supply : ON" << std.endl;
        int a = ad.execute_trajectory(values, ms *1000000);
        std::cout << "Neutral : OK " << std::endl;
        std:cout << "overruns : " << std::dec << a << std::endl;
        
        
        
        
        
        
        
        
        
        /*
        if(inputstate == "0") // if input pin is at state "0" i.e. button pressed
        {
            cout << "input pin state is \"Pressed \".n Will check input pin state again in 20ms "<<endl;
            usleep(20000);
            cout << "Checking again ....." << endl;
            gpio18->getval_gpio(inputstate); // checking again to ensure that state "0" is due to button press and not noise
            
            if(inputstate == "0")
            {

                cout << " Waiting until pin is unpressed....." << endl;
                
                cout << "pin is unpressed" << endl;
                
   

            }
            else
                cout << "input pin state is definitely \"UnPressed\". That was just noise." <<endl;

        }
        */
    }
    
    std::cout << "Neutral::Done." << std::endl;
    ad.spi_close();
    
    cout << "Exiting....." << endl;
    return 0;
}