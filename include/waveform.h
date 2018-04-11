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
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

#include "wav_file.h"

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
/* @struct motion
 * @brief Group of variables defining a motion
 */
struct motion
{
    std::vector<uint16_t>   motionVec;
    WavFile *               motionWAV;
    
    std::string     name;
    std::string     wav;
    
    variableMove    typeSignal;
    variableMove    amplitude;
    variableMove    duration;
    
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
    void configure(int nmessage_sec, bool useWAV);
    
    
    /**********************************************/
    /***            @brief Tools                ***/
    /**********************************************/
    bool insertMotion(struct motion motionVec);
    
    
    /**********************************************/
    /***            @brief Printers             ***/
    /**********************************************/
    void printCharacteristics();
    void print_appmoves();
    
    
    /**********************************************/
    /***            @brief Getters              ***/
    /**********************************************/
    int                     getFreqRefresh_mHz();
    int                     getMotionSize(std::string motionName);
    struct motion           getMotion(std::string motionName);
    std::vector<uint16_t>   getTapMove(actuator a);

    
    
    
private:
    /**********************************************/
    /***            @brief Private variables    ***/
    /**********************************************/
    // refresh rate bits/milliseconds (SPI, RPI3 to AD5383)
    int     freqRefresh_mHz;
    bool    useWAV;
    // Map of the motions
    std::map<std::string, motion>              motionsMap;
    std::map<std::string, motion>::iterator    it_motionsMap;
    
    
    /**********************************************/
    /***            @brief Private functions    ***/
    /**********************************************/
    bool extractWAV(struct motion * m);
    void create_moveWAV(struct motion * m);
    
    //void create_appMoveDefault();
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



