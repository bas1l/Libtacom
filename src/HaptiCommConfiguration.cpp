//----------------------------------------------------------------------
// Copyright 2011 Ciaran McHale.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions.
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.  
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//----------------------------------------------------------------------

#include "HaptiCommConfiguration.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using CONFIG4CPP_NAMESPACE::Configuration;
using CONFIG4CPP_NAMESPACE::ConfigurationException;

using namespace std;

//----------------------------------------------------------------------
// class HaptiCommConfigurationException
//----------------------------------------------------------------------

HaptiCommConfigurationException::HaptiCommConfigurationException(const char * str)
{
    m_str = new char[strlen(str) + 1];
    strcpy(m_str, str);
}


HaptiCommConfigurationException::HaptiCommConfigurationException(const HaptiCommConfigurationException & other)
{
    m_str = new char[strlen(other.m_str) + 1];
    strcpy(m_str, other.m_str);
}


HaptiCommConfigurationException::~HaptiCommConfigurationException()
{
    delete [] m_str;
}


const char * HaptiCommConfigurationException::c_str() const
{
    return m_str;
}



//----------------------------------------------------------------------
// class HaptiCommConfiguration
//----------------------------------------------------------------------

HaptiCommConfiguration::HaptiCommConfiguration()
{
    m_cfg = Configuration::create();
}


HaptiCommConfiguration::HaptiCommConfiguration(const char * cfgSource)
{
    m_cfg = Configuration::create();
    m_cfgSource = cfgSource;
}



HaptiCommConfiguration::~HaptiCommConfiguration()
{
    m_cfg->destroy();
}


void HaptiCommConfiguration::parse(const char * cfgSource, const char * scope) 
throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;

    m_scope = scope;
    try {
            if (cfgSource != 0 && strcmp(cfgSource, "") != 0) {
                m_cfgSource = cfgSource;
                m_cfg->parse(cfgSource);
            }
        //m_cfg->setFallbackConfiguration(m_cfg);
        //ConfigurationImpl::setFallbackConfiguration(Configuration * cfg)
        //cfg->setFallbackConfiguration(Configuration::INPUT_STRING, Configuration::getString());
        //cfg->setFallbackConfiguration(Configuration::INPUT_STRING, FallbackConfiguration::getString());
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}




//--------
// Configuration functions.


void HaptiCommConfiguration::configure(const char * cfgSource, DEVICE * dev, WAVEFORM * wf, ALPHABET * alph)
throw (HaptiCommConfigurationException)
{
    
    parse(cfgSource, "HaptiComm");
    configureDevice(dev);
    configureWaveform(wf);
    configureAlphabet(alph, dev, wf);
    
}

//--------
void HaptiCommConfiguration::configureDevice(DEVICE * dev)
throw (HaptiCommConfigurationException)
{
    StringBuffer __m_scope = m_scope;
    
    StringBuffer scope; 
    Configuration::mergeNames(m_scope.c_str(), "device.actuators", scope);
    StringBuffer filter;
    const char * id;
    int len;
    int i;
    
    m_scope = scope;
    try {
        int nbActuator = (int) m_cfg->lookupInt(m_scope.c_str(), "nbActuator");
        int actMaxValue = (int) m_cfg->lookupInt(m_scope.c_str(), "actuatorMaxValue");
        dev->setnbActuator(nbActuator);
        dev->setactMaxValue(actMaxValue);
        
        Configuration::mergeNames(scope.c_str(), "uid-actuator", filter);
        m_cfg->listFullyScopedNames(m_scope.c_str(), "", Configuration::CFG_SCOPE,
                                    false, filter.c_str(), m_scopeNames);
        len = m_scopeNames.length();
        printf("There are %d actuators\n", len);
        for (i = 0; i < len; i++) {
            
            id = lookupActuatorID(m_scopeNames[i]);
            struct actuator ac = lookupActuator(m_scopeNames[i]);
            
            dev->setActuator(ac, id);
        }
    }
    catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
    
    m_scope = __m_scope;
}




void HaptiCommConfiguration::configureWaveform(WAVEFORM * wf)
throw (HaptiCommConfigurationException)
{
    StringBuffer __m_scope = m_scope;
    
    StringBuffer scope; 
    Configuration::mergeNames(m_scope.c_str(), "waveform", scope);
    StringBuffer filter;
    const char * id;
    int len;
    int i;
    
    m_scope = scope;
    
    try {
        int freqRefresh_Hz = (int) m_cfg->lookupInt(scope.c_str(), "freqRefresh");
        bool useWAV = m_cfg->lookupBoolean(scope.c_str(), "useWAV");
        wf->configure(freqRefresh_Hz, useWAV);
        
        Configuration::mergeNames(scope.c_str(), "uid-motion", filter);
        m_cfg->listFullyScopedNames(m_scope.c_str(), "", Configuration::CFG_SCOPE,
                                    false, filter.c_str(), m_scopeNames);
        len = m_scopeNames.length();
        printf("There are %d Motion\n", len);
        for (i = 0; i < len; i++) {
            struct motion m = lookupMotion(m_scopeNames[i]);
            
            wf->insertMotion(m);
        }

    }
    catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
    
    m_scope = __m_scope;
}


void HaptiCommConfiguration::configureAlphabet(
                        ALPHABET * alph, 
                        DEVICE * dev, 
                        WAVEFORM * wf)
throw (HaptiCommConfigurationException)
{
    alph->configure(dev, wf);
    
    
    StringBuffer __m_scope = m_scope;
    
    StringBuffer scope; 
    Configuration::mergeNames(m_scope.c_str(), "alphabet", scope);
    StringBuffer filter;
    const char * id;
    int len;
    int i;
    
    m_scope = scope;
    
    try {
        Configuration::mergeNames(scope.c_str(), "uid-symbol", filter);
        m_cfg->listFullyScopedNames(m_scope.c_str(), "", Configuration::CFG_SCOPE,
                                    false, filter.c_str(), m_scopeNames);
        len = m_scopeNames.length();
        printf("There are %d symbols\n", len);
        for (i = 0; i < len; i++) {
            struct symbol s = lookupSymbol(m_scopeNames[i]);
            
            alph->insertSymbol(s);
        }

    }
    catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
    
    m_scope = __m_scope;
}



//--------  
// PRIVATE functions.
//--------

void HaptiCommConfiguration::initVariableMove(struct variableMove * vm) 
{
    vm->key = ' ';
    vm->name = "";
    vm->value = 2048;
    vm->valueDefault = 2048;
    vm->min = 0;
    vm->max = 4095;
}


//--------  
// Lookup-style functions.
//--------

struct actuator HaptiCommConfiguration::lookupActuator(const char * scopeActuator) 
const throw (HaptiCommConfigurationException){
    
    StringBuffer scopeActions;
    struct actuator ac = {};
    
    try {
        StringBuffer windingStr = m_cfg->lookupString(scopeActuator, "windingDirection");
        ac.windingDirection = (int8_t) (strcmp(windingStr.c_str(),"anticlockwise")?+1:-1); // /!\ if(0) => same string !
        ac.chan = (uint8_t) m_cfg->lookupInt(scopeActuator, "dacChannel");
        ac.name = m_cfg->lookupString(scopeActuator, "name");
        
        Configuration::mergeNames(scopeActuator, "actionValues", scopeActions);
        ac.vneutral = (uint16_t) m_cfg->lookupInt(scopeActions.c_str(), "neutral");
        ac.vpush = (uint16_t) m_cfg->lookupInt(scopeActions.c_str(), "push");
        ac.vup = (uint16_t) m_cfg->lookupInt(scopeActions.c_str(), "up");
    
        
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
    
    return ac;
}


const char * HaptiCommConfiguration::lookupActuatorID(const char * scopeActuator) 
const throw (HaptiCommConfigurationException)
{
    try {
        return m_cfg->lookupString(scopeActuator, "id");
    } catch(const ConfigurationException & ex) {
        throw HaptiCommConfigurationException(ex.c_str());
    }
}


struct motion HaptiCommConfiguration::lookupMotion(const char * scopeMotion) 
throw (HaptiCommConfigurationException)
{    
    struct motion * m = new motion();
    
    initVariableMove(&(m->typeSignal));
    initVariableMove(&(m->amplitude));
    initVariableMove(&(m->duration));
    
    try {
        m->name      = m_cfg->lookupString(scopeMotion, "name");
        m->wav       = m_cfg->lookupString(scopeMotion, "wav");
        
        m->duration.value  = m_cfg->lookupInt(scopeMotion, "duration");
        m->amplitude.value = m_cfg->lookupInt(scopeMotion, "amplitude");
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
    
    return (*m);
}

struct symbol HaptiCommConfiguration::lookupSymbol(const char * scopeSymbol) 
throw (HaptiCommConfigurationException)
{   
    struct symbol * s = new symbol();
    
    StringVector listID;
    StringBuffer lineID;
    StringBuffer nextLineID;
    StringBuffer currID;
    int lineInt;
    
    int i;
    
    try {
        s->name       = m_cfg->lookupString(scopeSymbol, "name");
        s->motion     = m_cfg->lookupString(scopeSymbol, "motion");
        s->actOverlap = m_cfg->lookupFloat(scopeSymbol, "actuatorOverlap");
    
        s->actArr.clear();
        m_cfg->lookupList(scopeSymbol, "actuatorArrangement", listID);
        
        lineID  = listID[0];
        lineInt = std::stoi(lineID.c_str());
        nextLineID = StringBuffer();
        nextLineID.append(lineInt+1);
        for(i=1; i<listID.length(); i++)
        {
            currID = listID[i];
            if (Configuration::patternMatch(currID.c_str(), nextLineID.c_str()))
            {
                lineInt     = std::stoi(nextLineID.c_str());
                nextLineID  = StringBuffer();
                nextLineID.append(lineInt+1);
            }
            else
            {
                s->actArr.push_back(actuatorStartLine(currID.c_str(), lineInt));
            }
        }
                
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
    
    return (*s);
}







void HaptiCommConfiguration::lookupList(const char * name, const char **& array, int & arraySize)
const throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;
    try {
            m_cfg->lookupList(m_scope.c_str(), name, array, arraySize);
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}



int HaptiCommConfiguration::lookupInt(const char * name) 
const throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;
    try {
            return m_cfg->lookupInt(m_scope.c_str(), name);
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}



float HaptiCommConfiguration::lookupFloat(const char * name) 
const throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;
    try {
            return m_cfg->lookupFloat(m_scope.c_str(), name);
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}



bool HaptiCommConfiguration::lookupBoolean(const char * name) 
const throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;
    try {
            return m_cfg->lookupBoolean(m_scope.c_str(), name);
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}



int HaptiCommConfiguration::lookupDurationMilliseconds(const char * name) 
const throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;
    try {
            return m_cfg->lookupDurationMilliseconds(m_scope.c_str(), name);
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}



int HaptiCommConfiguration::lookupDurationSeconds(const char * name) 
const throw (HaptiCommConfigurationException)
{
    //Configuration * cfg = (Configuration *)m_cfg;
    try {
            return m_cfg->lookupDurationSeconds(m_scope.c_str(), name);
    } catch(const ConfigurationException & ex) {
            throw HaptiCommConfigurationException(ex.c_str());
    }
}

