/*
 * waveform.h
 *
 *  Created on: 5 apr. 2016
 *  Author: basilou
 */
#ifndef WAVEFORM_H_
#define WAVEFORM_H_



#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

#include "utils.h"
#include "device.h"




struct variableMove
{
    std::string name;
    char key;
    int value;
    int valueDefault;
    int min;
    int max;
};// variableAppMove_default = {' ', -1, -1, -1};

struct moveWF
{
    std::string name;
    std::string wav;
    
    variableMove typeSignal;
    variableMove amplitude;
    variableMove duration;
};

struct appMove
{
    moveWF asc;
    moveWF action;
    
    variableMove nbAct;
    variableMove actCovering;
};




class WAVEFORM
{
public:
    WAVEFORM();
    WAVEFORM(int _tapmove_dur, int _appAscDuration, int _appActionDuration,
                int _appActionAmplitude, int _app_nb_act_simult);
    ~WAVEFORM();

    
    /**
     * @brief Initializes waveform properties with default parameters
     */ 
    void configure( int _freqRefresh, 
                    int _tapDuration, int _appActSuperposed, 
                    int _appRatioCover, 
                    int _appAscDuration, 
                    int _appActionDuration, int _appActionAmplitude);
    void configure(struct moveWF _tapmc, struct appMove _amc, int nmessage_sec, int useWAV);
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
    
    void printCharacteristics();
    /**
     * @brief print the values of the apparent movement
     */
    void print_appmoves();
    /**
     * @brief get the apparent movement pointer
     */
    int get_app_move_size();
    moveWF getTapMoveC();
    
    /**
     * @brief get the frequency of refresh for all channels (all actuator)
     */
    int get_freqRefresh_mHz();
    
    std::vector<uint16_t> getTapMove(actuator a);
    std::vector<uint16_t> getAppMove();
    
    void insert_app_move(actuator a, int start_at, std::vector<std::vector<uint16_t>>& result);
    void insert_tap_move(actuator a, bool push, std::vector<std::vector<uint16_t>>& result);
    
private:
    void create_tapMoveWAV();
    void create_appMoveWAV();
    void create_app_move_standard();
    float * create_envelope_sin(int length, int ampl);
    float * create_envelope_asc(int length);
    
    
    int freqRefresh_mHz;
    
    std::vector<uint16_t> tapMoveVec;
    std::vector<uint16_t> appMoveVec;
    
    struct 	moveWF tapmc;
    int     tapDuration;// 20 ms
    
    struct 	appMove amc;
    int     appDuration; //APPARENT_DURATION (APPARENT_ASC_DURATION+APPARENT_MOVE_DURATION)
    int     appActSuperposed; // 4
    float   appRatioCover; // .25
    int     appAscDuration;// 40 ms
    int     appAscAmplitude;// 40 ms
    int     appActionDuration;// 80 ms
    int     appActionAmplitude;// 700 unit of voltage (0 < v < 4095)
    
};
#endif // WAVEFORM_H_


