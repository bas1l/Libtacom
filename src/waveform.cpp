
    /*
 * waveform.cpp
 *
 *  Created on: 5 april. 2016
 *  Author: basilou
 */

#include "waveform.h"
#include  "wav_file.h"




WAVEFORM::WAVEFORM(
            int _tapDuration, int _appAscDuration, int _appActionDuration,
            int _appActionAmplitude, int _appActSuperposed) : 
    tapDuration(_tapDuration),
    appAscDuration(_appAscDuration),
    appActionDuration(_appActionDuration),
    appActionAmplitude(_appActionAmplitude),
    appActSuperposed(_appActSuperposed) {
    if (tapDuration < 3)
    {
        perror("TAP_MOVE_DURATION < 3 milliseconds is forbidden");
    }
    appDuration = appAscDuration + appActionDuration;
}


WAVEFORM::WAVEFORM() {}


WAVEFORM::~WAVEFORM() {}




void 
WAVEFORM::configure(int _freqRefresh_mHz, int _tapDuration, int _appActSuperposed, 
                    int _appRatioCover, int _appAscDuration, 
                    int _appActionDuration, int _appActionAmplitude)
{
    freqRefresh_mHz = _freqRefresh_mHz;

    tapDuration = _tapDuration;
    if (tapDuration < 3)
    {
        perror("TAP_MOVE_DURATION < 3 milliseconds is forbidden");
    }
    
    appActSuperposed    = _appActSuperposed;
    appRatioCover       = _appRatioCover/(float)100; //integer ratio to double
    appAscDuration      = _appAscDuration;
    appActionDuration   = _appActionDuration;
    appActionAmplitude  = _appActionAmplitude;
    appDuration         = appAscDuration + appActionDuration;
    
    create_app_move_standard();
}


void 
WAVEFORM::configure(int _tapDuration, struct appMove _amc, int nmessage_Hz, int useWAV)
{
    freqRefresh_mHz = nmessage_Hz * (1/(double)sec2ms);

    tapDuration = _tapDuration;
    if (tapDuration < 3)
    {
        perror("TAP_MOVE_DURATION < 3 milliseconds is forbidden");
    }
    
    appActSuperposed    = _amc.nbAct.value;
    appRatioCover       = _amc.actCovering.value/(float)100; //integer ratio to double
    appAscDuration      = _amc.asc.duration.value;
    appAscAmplitude	= _amc.asc.amplitude.value;
    appActionDuration   = _amc.action.duration.value;
    appActionAmplitude  = _amc.action.amplitude.value;
    amc = _amc;
    
    if (useWAV){
        create_appMoveWAV();
    }
    else{
        create_app_move_standard();
    }
}


void 
WAVEFORM::configure()
{
    create_app_move_standard();
}


void 
WAVEFORM::insert_tap_move(actuator a, bool push, std::vector<std::vector<uint16_t>>& result)
{
    int nbValue = freqRefresh_mHz*tapDuration; //tapdur=millisec
    
    int end = nbValue-3;
    uint16_t vneutral = (uint16_t) ~((unsigned int) a.vneutral);
    int ms;
    
    if (push)
    {// if the movement is to push
        uint16_t vpush = (uint16_t) ~((unsigned int) a.vpush);
        uint16_t vpushb = (uint16_t) ~((unsigned int) 4095-a.vpush);//ACTUATOR_MAXVALUE-a.vpush);
        
        // push the actuator
        for(ms=0; ms<end; ms++)
        {
            result[a.chan].push_back(vpush);
        }
        // recall the actuator
        for(; ms<end-1; ms++)
        {
            result[a.chan].push_back(vpushb);
        }
        // put the neutral value at the end
        result[a.chan].push_back(vneutral);
    }
    else
    {// if the actuator is not a part of the movement
        for(ms=0; ms<nbValue; ms++)
        {
            result[a.chan].push_back(vneutral);
        }
    }
}


/*
 *  private :
 */
void 
WAVEFORM::create_appMoveWAV()
{
    int         t           = 0;
    //uint16_t    inv         = 4095;
    WavFile*    pObjWavFile = new WavFile();
    
    appMoveVec.clear();
    
    
    // part 1/2 : ascension 
    int     offset  = amc.asc.amplitude.value;
    int     nbValue = freqRefresh_mHz * appAscDuration; //appAscDuration=millisec
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
    if (EXIT_SUCCESS != pObjWavFile->openWavFile(cstr))
    {
        std::cout<<"\nCan't load wav file.";
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
        appMoveVec.push_back((uint16_t) (offset+ ratio*samplesWAV[(int)(t*incr)]));
    }
    
    free(cstr);
}


void 
WAVEFORM::create_app_move_standard()
{
    int t;
    uint16_t inv = 4095;
    appMoveVec.clear();
 
    // part 1/2 : ascension 
    int nbValue = freqRefresh_mHz*appAscDuration; //appAscDuration=millisec
    float * asc = create_envelope_asc(nbValue);

    for (t=0; t<nbValue; t++)
    {
            appMoveVec.push_back(inv - (uint16_t) (2000+1000*asc[t]));
            //printw("asc=%.f", asc[t]);
    }


    // part 2/2 : pink noise 
    nbValue = freqRefresh_mHz*appActionDuration; //appAscDuration=millisec
    float * shape = create_envelope_sin(nbValue, appActionAmplitude);
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


void 
WAVEFORM::insert_app_move(actuator a, int start_at, std::vector<std::vector<uint16_t>>& result)
{// actuator a est a utiliser quand on voudra affiner le lever de tige
    result[a.chan].insert(result[a.chan].begin()+start_at, 
                          appMoveVec.begin(), appMoveVec.end());
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
    std::cout << "Waveform characteristics (ms and numeric values):\n"
            << "Tap movement:\n"
                << "\tDuration=" << tapDuration << "\n"
            << "Apparent movement:\n"
                << "\tTotal duration=" << appDuration << "\n"
                << "\tNumber of actuators=" << amc.nbAct.value << "\n"
                << "\tValue of covering=" << amc.actCovering.value << "\n"
                << "\t1st part: go up movement:\n"
                    << "\t\tDuration=" << amc.asc.duration.value << "\n"
                    << "\t\tFinal value=" << amc.asc.amplitude.value << "\n"
                << "\t2nd part: action movement:\n"
                    << "\t\tDuration=" << amc.action.duration.value << "\n"
                    << "\t\tAmplitude=" << amc.action.amplitude.value << "\n"
    << std::endl;
}

void 
WAVEFORM::print_appmoves()
{
    for (std::vector<uint16_t>::const_iterator i = appMoveVec.begin(); i != appMoveVec.end(); ++i)
    {
        std::cout << "app_move : " << *i << std::endl;
    }
}
