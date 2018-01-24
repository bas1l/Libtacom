/*
 * waveform.h
 *
 *  Created on: 5 apr. 2016
 *  Author: basilou
 */
#ifndef WAVEFORM_H_
#define WAVEFORM_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include "utils.h"
#include "waveform_defines.h"
#include "device.h"


class WAVEFORM
{
public:
    WAVEFORM(int _tapmove_dur, int _appasc_dur, int _appmove_dur,
                int _appmove_ampl, int _app_nb_act_simult);
    
    WAVEFORM();
    ~WAVEFORM();

    
    /**
     * @brief Initializes waveform properties with default parameters
     */ 
    void configure(int _tapmove_dur, int _appasc_dur, int _appmove_dur,
                int _appmove_ampl, int _app_nb_act_simult);
    /**
     * @brief Initializes waveform properties with default parameters
     */
    void configure();

    /**
     * @brief create a sin with all the needed parameters
     * @param freq frequency of the sine
     * @param ampl amplitude of the sine
     * @param phase phase of the sine
     * @param nsample number of sample requiered 
     * @param offset offset of the sine
     */
    uint16_t * create_sin(int freq, int ampl, int phase, int nsample, int offset);
    
    /**
     * @brief create an array of nsample random values between a and b
     * @param nsample number of sample requiered
     * @param a minimal value
     * @param b maximal value
     */
    float * create_random_dots(int nsample, int a, int b);
    
    /**
     * @brief print the values of the apparent movement
     */
    void print_appmoves();
    /**
     * @brief get the apparent movement pointer
     */
    int get_app_move_size();
    
    void create_app_move(actuator a, int start_at, std::vector<std::vector<uint16_t>>& result);
    void create_tap_move(actuator a, bool push, std::vector<std::vector<uint16_t>>& result);
    void create_tap_move_bis(actuator a, bool push, std::vector<uint16_t>& result);
    
private:
    void create_app_move_standard();
    float * create_envelope_sin(int length, int ampl);
    float * create_envelope_asc();
    
    
    std::vector<uint16_t> apparent_move;
    
    //float deg2rad = 3.14159265/180;

    int tapmove_dur;// 20 ms
    int appasc_dur;// 40 ms
    int appmove_dur;// 80 ms
    int app_tot_dur; //APPARENT_DURATION (APPARENT_ASC_DURATION+APPARENT_MOVE_DURATION)
    int appmove_ampl;// 700 unit of voltage (0 < v < 4095)
    int app_nb_act_simult; // 4
};
#endif // WAVEFORM_H_


