

/*
 * actuator.h
 *
 *  Created on: 7 apr. 2016
 *      Author: basilou
 */ 
 
#ifndef H_DEVICE_H_
#define H_DEVICE_H_

#include <map>
#include <queue>
#include <stdio.h>
#include <vector>

#include "device_defines.h"

struct actuator
{ 
    std::string name;
    uint8_t chan; 
    uint16_t vup; 
    uint16_t vneutral; 
    uint16_t vpush;
};

class DEVICE
{
public :
    std::map<std::string, struct actuator> actuators;
    // constructors
    DEVICE();

    ~DEVICE();

    /*
     * @brief configure the actuators values and the letters
     *        depending on the actuator_define
     */
    void configure();


    /*
     * @brief return the values according to the actuator's name
     * @param n name of the actuator requested
     */
    struct actuator getact(std::string n);


    /*
     * @brief say if actuators is empty
     */
    bool empty();
	
private:
    actuator make_actuator(std::string n, uint8_t _chan, uint16_t _vup, 
                           uint16_t _vneutral, uint16_t _vpush);
    void configure_actuators();
    
    std::map<std::string, struct actuator>::iterator it_act;
};

#endif //H_DEVICE_H

