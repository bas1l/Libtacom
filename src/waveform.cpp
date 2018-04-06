
    /*
 * waveform.cpp
 *
 *  Created on: 5 april. 2016
 *  Author: basilou
 */

#include "waveform.h"
#include  "wav_file.h"


WAVEFORM::WAVEFORM() {}

WAVEFORM::~WAVEFORM() {}


void 
WAVEFORM::configure()
{
    create_app_move_default();
}

void 
WAVEFORM::configure(struct moveWF _tapmc, struct appMove _amc, int nmessage_Hz, int useWAV)
{
    freqRefresh_mHz = nmessage_Hz * (1/(double)sec2ms);
    
    if (_tapmc.duration.value < 3)
    {
        perror("TAP_MOVE_DURATION < 3 milliseconds is forbidden");
    }
    
    tapmc = _tapmc;
    amc = _amc;
    
    
    if (useWAV){
        create_appMoveWAV();
        create_tapMoveWAV();
    }
    else{
        create_app_move_default();
    }
}






std::vector<uint16_t>
WAVEFORM::getTapMove()
{// actuator a est a utiliser quand on voudra affiner le lever de tige
    return tapMoveVec;
}


std::vector<uint16_t> WAVEFORM::getTapMove(actuator a)
{
    int ms;
    int nbValue = freqRefresh_mHz*tapmc.duration.value; //409;//tapdur=millisec
    
    int nbNeutral   = 1;
    int nbBackPush  = 2;
    int nbPush      = nbValue-(nbBackPush+nbNeutral);
    
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
    for(ms=0; ms<nbBackPush; ms++)
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

void 
WAVEFORM::insert_tap_move(actuator a, bool push, std::vector<std::vector<uint16_t>>& result)
{
    int ms;
    int         nbValue     = freqRefresh_mHz*tapmc.duration.value; //409;//tapdur=millisec
    uint16_t    vneutral    = (uint16_t) ~((unsigned int) a.vneutral);
    
    if (push)
    {// if the movement is to push
        int nbNeutral = 1;
        int nbBackPush = 2;
        int nbPush = nbValue-(nbBackPush+nbNeutral);
        uint16_t vpush = (uint16_t) ~((unsigned int) a.vpush);
        uint16_t vpushb = (uint16_t) ~((unsigned int) 4095-a.vpush);//ACTUATOR_MAXVALUE-a.vpush);
        
        // push the actuator
        for(ms=0; ms<nbPush; ms++)
        {
            result[a.chan].push_back(vpush);
        }
        // recall the actuator
        for(ms=0; ms<nbBackPush; ms++)
        {
            result[a.chan].push_back(vpushb);
        }
        // put the neutral value at the end
        for(ms=0; ms<nbNeutral; ms++)
        {
            result[a.chan].push_back(vneutral);
        }
    }
    else
    {// if the actuator is not a part of the movement
        for(ms=0; ms<nbValue; ms++)
        {
            result[a.chan].push_back(vneutral);
        }
    }
}



std::vector<uint16_t>
WAVEFORM::getAppMove()
{// actuator a est a utiliser quand on voudra affiner le lever de tige
    return appMoveVec;
}



void 
WAVEFORM::insert_app_move(actuator a, int start_at, std::vector<std::vector<uint16_t>>& result)
{// actuator a est a utiliser quand on voudra affiner le lever de tige
    result[a.chan].insert(result[a.chan].begin()+start_at, 
                          appMoveVec.begin(), appMoveVec.end());
}



/*  PRIVATE:
 * 
 * 
 * 
 * 
 */
void 
WAVEFORM::create_tapMoveWAV()
{
    int t = 0;
    WavFile* pObjWavFile = new WavFile();
    tapMoveVec.clear();
    
    
    // Extract the WAV
    char *cstr = new char[tapmc.wav.length() + 1];
    strcpy(cstr, tapmc.wav.c_str());
    // open the corresponding file
	//std::cout<<"\nfile:"<< tapmc.wav << std::endl;
    if (EXIT_SUCCESS != pObjWavFile->openWavFile(cstr))
    {
        std::cout<<"\nCan't load wav file:"<< cstr << endl;
        exit(-1);
    }
    // Compensate the difference between WAV and DAC frequencies
    double* samplesWAV  = pObjWavFile->getData();
    int     nbSamples   = pObjWavFile->getNumSamples();
    int     fs_mHz      = (int)(pObjWavFile->getSampleRateHz()*(1/(double)sec2ms));
    double  incr        = fs_mHz / (double) freqRefresh_mHz;
    int 	nbValue 	= (int) (nbSamples/incr);
    
    // Compensate the difference between WAV and DAC amplitudes
    int amplitude   = tapmc.amplitude.value;
    double minAbs   = abs(*std::min_element(samplesWAV, samplesWAV+nbSamples));
    double max      = *std::max_element(samplesWAV, samplesWAV+nbSamples);
    double ratio    = amplitude/ (minAbs>max?minAbs:max);
    
    // write the movement:
    for (t=0; t<nbValue; t++){
        tapMoveVec.push_back((uint16_t) (4095 - ratio*samplesWAV[(int)(t*incr)]));
    }
    
    free(cstr);
}


void 
WAVEFORM::create_appMoveWAV()
{
    int         t           = 0;
    //uint16_t    inv         = 4095;
    WavFile*    pObjWavFile = new WavFile();
    
    appMoveVec.clear();
    
    
    // part 1/2 : ascension 
    int     offset  = amc.asc.amplitude.value;
    int     nbValue = freqRefresh_mHz * amc.asc.duration.value; //appAscDuration=millisec
    float*  asc     = create_envelope_asc(nbValue);
    
    
    int vup = offset - 2048;
    for (t=0; t<nbValue; t++)
    {
            appMoveVec.push_back((uint16_t) (2048+vup*asc[t]));
            //printw("asc=%.f", asc[t]);
    }

    
    // part 2/2 : pink noise 
    // Extract the WAV
    char *cstr = new char[amc.action.wav.length() + 1];
    strcpy(cstr, amc.action.wav.c_str());
    // open the corresponding file
	//std::cout<<"\nfile:"<< amc.action.wav << std::endl;
    if (EXIT_SUCCESS != pObjWavFile->openWavFile(cstr))
    {
        std::cout<<"\nCan't load wav file:" << cstr << endl;
        exit(-1);
    }
    
    // Compensate the difference between WAV and DAC frequencies
    double* samplesWAV  = pObjWavFile->getData();
    int     nbSamples   = pObjWavFile->getNumSamples();
    int     fs_mHz      = (int)(pObjWavFile->getSampleRateHz()*(1/(double)sec2ms));
    double  incr        = fs_mHz / (double) freqRefresh_mHz;
    nbValue = (int) (nbSamples/incr);
    
    // Compensate the difference between WAV and DAC amplitudes
    int amplitude   = amc.action.amplitude.value;
    double min      = abs(*std::min_element(samplesWAV, samplesWAV+nbSamples));
    double max      = *std::max_element(samplesWAV, samplesWAV+nbSamples);
    double ratio    = amplitude/ (min>max?min:max);
    
    // write the movement:
    for (t=0; t<nbValue; t++){
        appMoveVec.push_back((uint16_t) (4095 - offset+ ratio*samplesWAV[(int)(t*incr)]));
    }
    
    free(cstr);
}


void 
WAVEFORM::create_app_move_default()
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
            //printw("asc=%.f", asc[t]);
    }


    // part 2/2 : pink noise 
    nbValue = freqRefresh_mHz*amc.action.duration.value; //appAscDuration=millisec
    float * shape = create_envelope_sin(nbValue, amc.action.amplitude.value);
    float * r = create_random_dots(nbValue, -1, 1);

    for (t=0; t<nbValue; t++){
            appMoveVec.push_back(inv - (uint16_t)(3200+ shape[t] * r[t]));
            //printw("move=%f", shape[t] * r[t]);
    }

    free(asc);
    free(shape);
    free(r);
}


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


moveWF WAVEFORM::getTapMoveC()
{
	return tapmc;	
}

int 
WAVEFORM::getAppOverlap()
{
	return amc.actOverlap.value; // value in pourcent (-> INTEGER)
}
    
int 
WAVEFORM::get_app_move_size()
{
    return appMoveVec.size();
}

int 
WAVEFORM::get_freqRefresh_mHz()
{
    return freqRefresh_mHz;
}


void 
WAVEFORM::printCharacteristics()
{
    std::cout << "_______________________________________________________\n" 
            << "Waveform characteristics:\n"
            << "Frequency of Refresh (AD5383)=" << freqRefresh_mHz << "mHz\n\n"
            << "<Tap movement>\n"
                << "\t-Duration=" << tapmc.duration.value << "ms\n"
            << "<Apparent movement>\n"
                << "\t-Total duration=" << amc.asc.duration.value+amc.action.duration.value << "ms\n"
                << "\t-Number of actuators=" << amc.nbAct.value << "\n"
                << "\t-Value of covering=" << amc.actOverlap.value << "%\n"
                << "\t[1st part: go up movement]\n"
                    << "\t\t-Duration=" << amc.asc.duration.value << "ms\n"
                    << "\t\t-Final value=" << amc.asc.amplitude.value << "#Numerical-AD5383\n"
                << "\t[2nd part: action movement]\n"
                    << "\t\t-Duration=" << amc.action.duration.value << "ms\n"
                    << "\t\t-Amplitude=" << amc.action.amplitude.value << "#Numerical-AD5383\n"
    << "_______________________________________________________" << std::endl;
}

void 
WAVEFORM::print_appmoves()
{
    for (std::vector<uint16_t>::const_iterator i = appMoveVec.begin(); i != appMoveVec.end(); ++i)
    {
        std::cout << "app_move : " << *i << std::endl;
    }
}
