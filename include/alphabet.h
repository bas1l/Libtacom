/*
 * alphabet.h
 *
 *  Created on: 7 apr. 2016
 *      Author: basilou
 */ 
 
#ifndef H_ALPHABET_H_
#define H_ALPHABET_H_

#include <algorithm>
#include <map>
#include <queue>
#include <stdio.h>
#include <string>
#include <vector>

#include "ad5383.h"
#include "waveform.h"
#include "device.h"

#include <iostream>

using namespace std;




/**********************************************/
/***            @typedef alphabet           ***/
/**********************************************/
typedef std::vector<std::vector<std::string>> actuatorsID;
/* @typedef actuatorPos & actuatorArrangement
 * @brief Arrangement of the actuator during a symbol
 * 
 */
typedef std::pair<std::string, int>     actuatorStartLine;
typedef std::vector<actuatorStartLine>  actuatorArrangement;

/* waveformLetter:
 * First -> Identification Number for ad5383 channels
 * Second -> Corresponding waveform values 
 */
typedef map<uint8_t, vector<uint16_t>> waveformLetter;


/**********************************************/
/***            @struct alphabet            ***/
/**********************************************/
/* @struct symbol
 * @brief Value of a symbol : 
 *          - its name
 *          - its timer of the used actuators
 *          - its type of motion
 */

struct symbol
{
    std::string name;
    std::string motion;
    double      actOverlap;
    actuatorArrangement actArr;
};




/**********************************************/
/***            @class ALPHABET             ***/
/**********************************************/
class ALPHABET
{
public :
    // constructors
    ALPHABET();
    ALPHABET(DEVICE * _dev, WAVEFORM * _wf);

    ~ALPHABET();
    
    /*
     * @brief configure the actuators values and the letters
     *        depending on the alphabet_define
     */
    void configure();
    void configure(DEVICE * _dev, WAVEFORM * _wf);
    
    
    std::string getlistSymbols();
    std::vector<std::vector<uint16_t>> getneutral();
    
    /*
     * @brief return the array according to the letter
     * @param letter letter requested
     */
    waveformLetter getl(std::string l);
    waveformLetter getl(char l);
    
    /*
     * @brief return the frequency with which the actuators are refreshed (Hz)
     */
    int getFreqRefresh_mHz();
    
    bool insertSymbol(struct symbol s);
    waveformLetter make_tapHoldLetter(std::vector<std::vector<std::string>> a_names);
    waveformLetter make_tapLetter(std::vector<std::string> a_names);
    waveformLetter make_appLetter(std::vector<std::vector<std::string>> a_names);
    
private:
    int getMaxStartLineID(actuatorArrangement * ar);
    waveformLetter make_letter(char l);
    bool configure_letters();
    void configure_neutral();
    
    DEVICE * dev;
    WAVEFORM * wf;
    
    std::string listSymbols;
    std::vector<std::vector<uint16_t>> neutral_statement;
    std::map<std::string, waveformLetter> dictionnary;
    std::map<std::string, waveformLetter >::iterator it_dictionnary;
};
#endif //H_ALPHABET_H

 