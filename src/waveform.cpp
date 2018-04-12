/*
 * waveform.cpp
 *
 *  Created on: 5 april. 2016
 *  Author: basilou
 */

#include "waveform.h"


WAVEFORM::WAVEFORM() {}

WAVEFORM::~WAVEFORM() {}


void 
WAVEFORM::configure(int nmessage_Hz, bool _useWAV)
{
    freqRefresh_mHz = nmessage_Hz * (1/(double)sec2ms);
    useWAV = _useWAV;
}


bool 
WAVEFORM::insertMotion(struct motion m)
{
    std::pair<std::map<std::string, motion>::iterator,bool> ret;
    std::pair<std::string, motion> result;
    
    
    if (useWAV)
    {
        extractWAV(&m);
        create_moveWAV(&m);
        
        cout    << "motion name:" << m.name 
            << ". wav:" << m.wav 
            << ". motionVec.size:" << m.motionVec.size() 
            << std::endl;
    }
    
    result  = std::make_pair (m.name, m);
    ret     = motionsMap.insert(result);
    
    return ret.second;
}


int
WAVEFORM::getMotionSize(std::string motionName)
{
    it_motionsMap = motionsMap.find(motionName);
    if (it_motionsMap == motionsMap.end())
    {
        std::cout<<"\nCan't get the motion:" << motionName << endl;
        exit(-1);
        
    }
    
    return it_motionsMap->second.motionVec.size();
}


struct motion
WAVEFORM::getMotion(std::string motionName)
{
    it_motionsMap = motionsMap.find(motionName);
    if (it_motionsMap == motionsMap.end())
    {
        std::cout<<"\nCan't get the motion:" << motionName << endl;
        exit(-1);
        
    }
    
    return it_motionsMap->second;
}



std::vector<uint16_t> WAVEFORM::getTapMove(actuator a)
{
    motion tapMotion = getMotion("tap");
    int ms;
    int nbValue = freqRefresh_mHz*tapMotion.duration.value; //409;//tapdur=millisec
    
    int nbNeutral = 1;
    int nbBack    = 2;
    int nbPush    = nbValue-(nbNeutral+nbBack);
    
    uint16_t vpush      = (uint16_t) ~((unsigned int) a.vpush);
    uint16_t vpushb     = (uint16_t) ~((unsigned int) 4095-a.vpush);//ACTUATOR_MAXVALUE-a.vpush);
    uint16_t vneutral   = (uint16_t) ~((unsigned int) a.vneutral);
   
    std::vector<uint16_t> result;
    result.reserve(nbValue);
   
    // push the actuator
    for(ms=0; ms<nbPush; ms++)
    {
        result.push_back(vpush);
    }
    // recall the actuator
    for(ms=0; ms<nbBack; ms++)
    {
        result.push_back(vpushb);
    }
    // put the neutral value at the end
    for(ms=0; ms<nbNeutral; ms++)
    {
        result.push_back(vneutral);
    }
    
    return result;
}



/*  PRIVATE:
 * 
 * 
 * 
 * 
 */
void 
WAVEFORM::create_moveWAV(struct motion * m)
{
    int t = 0;
    m->motionVec.clear();
    
    // Compensate the difference between WAV and DAC frequencies
    double * datas      = m->motionWAV->getData();
    int      nbSamples  = m->motionWAV->getNumSamples();
    int      fs_mHz     = (int)(m->motionWAV->getSampleRateHz()*(1/(double)sec2ms));
    double   incr       = fs_mHz / (double) freqRefresh_mHz;
    int      nbValue 	= (int) (nbSamples/incr);
    
    // Compensate the difference between WAV and DAC amplitudes
    int amplitude   = m->amplitude.value;
    double minAbs   = abs(*std::min_element(datas, datas+nbSamples));
    double max      = *std::max_element(datas, datas+nbSamples);
    double ratio    = amplitude/ (minAbs>max?minAbs:max);
    
    // write the movement:
    for (t=0; t<nbValue; t++){
        m->motionVec.push_back((uint16_t) (4095 - ratio*datas[(int)(t*incr)]));
    }
    
    m->motionVec.push_back((uint16_t) 2048);
}


/*
void 
WAVEFORM::create_appMoveWAV()
{
    int         t           = 0;
    uint16_t    inv         = 4095;
    
    appMoveVec.clear();
    
    // part 1/2 : ascension 
    int     offset  = amc.asc.amplitude.value;
    int     nbValue = freqRefresh_mHz * amc.asc.duration.value; //appAscDuration=millisec
    float*  asc     = create_envelope_asc(nbValue);
    int     vup     = offset - 2048;
    for (t=0; t<nbValue; t++)
    {
            appMoveVec.push_back((uint16_t) (2048+vup*asc[t]));
            //printw("asc=%.f", asc[t]);
    }

    
    // part 2/2 : 
    double * datas      = appMoveWAV->getData();
    int      nbSamples  = appMoveWAV->getNumSamples();
    int      fs_mHz     = (int)(appMoveWAV->getSampleRateHz()*(1/(double)sec2ms));
    double   incr       = fs_mHz / (double) freqRefresh_mHz;
    nbValue = (int) (nbSamples/incr);
    // Compensate the difference between WAV and DAC amplitudes
    int amplitude   = amc.action.amplitude.value;
    double min      = abs(*std::min_element(datas, datas+nbSamples));
    double max      = *std::max_element(datas, datas+nbSamples);
    double ratio    = amplitude/ (min>max?min:max);
    
    // write the movement:
    for (t=0; t<nbValue; t++){
        appMoveVec.push_back((uint16_t) (inv - (offset+ ratio*datas[(int)(t*incr)])));
    }
    
    appMoveVec.push_back((uint16_t) 2048);
}


void 
WAVEFORM::create_appMoveDefault()
{
    int t;
    uint16_t inv = 4095;
    appMoveVec.clear();
 
    // part 1/2 : ascension 
    int nbValue = freqRefresh_mHz*amc.asc.duration.value; //appAscDuration=millisec
    float * asc = create_envelope_asc(nbValue);
    for (t=0; t<nbValue; t++)
    {
            appMoveVec.push_back(inv - (uint16_t) (2000+1000*asc[t]));
    }
    
    // part 2/2 : pink noise 
    nbValue         = freqRefresh_mHz*amc.action.duration.value; //appAscDuration=millisec
    float * shape   = create_envelope_sin(nbValue, amc.action.amplitude.value);
    float * r       = create_random_dots(nbValue, -1, 1);
    for (t=0; t<nbValue; t++){
        appMoveVec.push_back(inv - (uint16_t)(3200+ shape[t] * r[t]));
    }

    free(asc);
    free(shape);
    free(r);
}

*/

float * 
WAVEFORM::create_envelope_sin(int length, int ampl)
{
    float * s;
    s = (float*) malloc(length * sizeof(float));

    float incr = M_PI/(float)length;

    for (int i=0; i<length; i++){
            s[i] = ampl *( sin(i * incr)*0.5 + 0.5);
    }

    return s;
}

float * 
WAVEFORM::create_envelope_asc(int length)
{
    float * s;
    s = (float*) malloc(length * sizeof(float));
    
    float incr = M_PI/(float)(2*length);

    for (int i=0; i<length; i++){
            s[i] = sin(i * incr);
    }

    return s;
}





uint16_t * 
WAVEFORM::create_sin(int freq, int ampl, int phase, int nsample, int offset)
{
    uint16_t * s;
    s = (uint16_t*) malloc(nsample * sizeof(uint16_t));

    // Suivant le nombre d'échantillons voulus :
    float incr = 2*M_PI/((float)nsample);
    for (int i=0; i<nsample; i++){
            s[i] = (uint16_t) floor(ampl * sin(i*incr*freq +phase) + offset);
    }
    return s;
}


float * 
WAVEFORM::create_random_dots(int nsample, int a, int b)
{
    float * r;
    r = (float*) malloc(nsample*sizeof(float));

    for (int t=0; t<nsample; t++){
            r[t] = rand_a_b(a, b);
    }

    return r;
}



double 
WAVEFORM::getFreqRefresh_mHz()
{
    return freqRefresh_mHz;
}


void 
WAVEFORM::printCharacteristics()
{
    /*
    std::cout << "_______________________________________________________\n" 
            << "Waveform characteristics:\n"
            << "Frequency of Refresh (AD5383)=" << freqRefresh_mHz << "mHz\n\n"
            << "<Tap movement>\n"
                << "\t-Duration=" << tapmc.duration.value << "ms\n"
            << "<Apparent movement>\n"
                << "\t-Total duration=" << amc.asc.duration.value+amc.action.duration.value << "ms\n"
                << "\t[1st part: go up movement]\n"
                    << "\t\t-Duration=" << amc.asc.duration.value << "ms\n"
                    << "\t\t-Final value=" << amc.asc.amplitude.value << "#Numerical-AD5383\n"
                << "\t[2nd part: action movement]\n"
                    << "\t\t-Duration=" << amc.action.duration.value << "ms\n"
                    << "\t\t-Amplitude=" << amc.action.amplitude.value << "#Numerical-AD5383\n"
    << "_______________________________________________________" << std::endl;
    */
}

void 
WAVEFORM::print_appmoves()
{
    /*
    for (std::vector<uint16_t>::const_iterator i = appMoveVec.begin(); i != appMoveVec.end(); ++i)
    {
        std::cout << "app_move : " << *i << std::endl;
    }
     */
}


bool 
WAVEFORM::extractWAV(struct motion * m)
{   
    m->motionWAV = new WavFile();
    // Extract the WAV
    char *cstr = new char[m->wav.length() + 1];
    strcpy(cstr, m->wav.c_str());
    // open the corresponding file
	//std::cout<<"\nfile:"<< m->wav << std::endl;
    if (EXIT_SUCCESS != m->motionWAV->openWavFile(cstr))
    {
        std::cout<<"\nCan't load wav file:"<< m->wav << endl;
        exit(-1);
    }
    free(cstr);

    return true;
}

