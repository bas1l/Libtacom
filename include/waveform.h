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

#include  "wav_file.h"

#include "utils.h"
#include "device.h"




/**********************************************/
/***            @struct motions             ***/
/**********************************************/
/* @struct VariableMove
 * @brief Informations of motion's variable
 */
struct variableMove
{
    std::string name;
    char key;
    int value;
    int valueDefault;
    int min;
    int max;
};
/* @struct moveWF
 * @brief Group of variables defining a motion
 */
struct moveWF
{
    std::string name;
    std::string wav;
    
    variableMove typeSignal;
    variableMove amplitude;
    variableMove duration;
};
/* @struct appMove
 * @brief specific structure defining the apparent motion
 */
struct appMove
{
    moveWF asc;
    moveWF action;
    
    variableMove nbAct;
    variableMove actOverlap;
};
/* @struct tapHoldMove
 * @brief specific structure defining the tapHold motion
 */
struct tapHoldMove
{
    moveWF action;
    
    variableMove nbAct;
    variableMove actOverlap;
};




/**********************************************/
/***            @class WAVEFORM             ***/
/**********************************************/
class WAVEFORM
{
public:
    /**********************************************/
    /***            @brief (Cons/Des)tructor    ***/
    /**********************************************/
    WAVEFORM();
    ~WAVEFORM();
    
    
    /**********************************************/
    /***            @brief Configurations       ***/
    /**********************************************/
    void configure( struct moveWF _tapHoldmc,
                    struct moveWF _tapmc, 
                    struct appMove _amc, 
                    int nmessage_sec, int useWAV);
    void configure();
    
    
    /**********************************************/
    /***            @brief Tools                ***/
    /**********************************************/
    std::vector<uint16_t> getTapMove(actuator a);
    
    
    /**********************************************/
    /***            @brief Printers             ***/
    /**********************************************/
    void printCharacteristics();
    void print_appmoves();
    
    
    /**********************************************/
    /***            @brief Getters              ***/
    /**********************************************/
    int                     getFreqRefresh_mHz();
    int                     getAppMoveSize();
    int                     getAppOverlap();
    moveWF                  getTapHoldMoveC();
    moveWF                  getTapMoveC();
    
    std::vector<uint16_t>   getTapHoldMove();
    std::vector<uint16_t>   getTapMove();
    std::vector<uint16_t>   getAppMove();
    
    
    
private:
    /**********************************************/
    /***            @brief Private variables    ***/
    /**********************************************/
    // refresh rate bits/milliseconds (SPI, RPI3 to AD5383)
    int freqRefresh_mHz;
    // Caracteristics of the motions
    struct moveWF  tapHoldmc;
    struct moveWF  tapmc;
    struct appMove amc;
    // Normalised values of the motions
    std::vector<uint16_t> tapHoldMoveVec;
    std::vector<uint16_t> tapMoveVec;
    std::vector<uint16_t> appMoveVec;
    // if WAV files exist: wav containers of the motions
    WavFile * tapHoldMoveWAV;
    WavFile * tapMoveWAV;
    WavFile * appMoveWAV;
    
    
    /**********************************************/
    /***            @brief Private functions    ***/
    /**********************************************/
    bool extractWAV();
    void create_tapHoldMoveWAV();
    void create_tapMoveWAV();
    void create_appMoveWAV();
    
    void create_appMoveDefault();
    float * create_envelope_sin(int length, int ampl);
    float * create_envelope_asc(int length);
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
};
#endif // WAVEFORM_H_


