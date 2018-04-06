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


/* waveformLetter:
 * First -> Identification Number for ad5383 channels
 * Second -> Corresponding waveform values 
 */
typedef map<uint8_t, vector<uint16_t>> waveformLetter;


/* class ALPHABET:
 * Regroup the functions to create the tactile deafblind alphabet.
 * 
 */
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
    waveformLetter getl(char l);
    
    /*
     * @brief return the frequency with which the actuators are refreshed (Hz)
     */
    int getFreqRefresh_mHz();

    waveformLetter make_app_letter(std::vector<std::vector<std::string>> a_names);
    waveformLetter make_tap_letter(std::vector<std::string> a_names);
    
private:
    waveformLetter make_letter(char l);
    bool configure_letters();
    void configure_neutral();
    
    DEVICE * dev;
    WAVEFORM * wf;
    
    string listSymbols;
    vector<vector<uint16_t>> neutral_statement;
    map<char, waveformLetter> letters;
    map<char, waveformLetter >::iterator it_letter;
};
#endif //H_ALPHABET_H

