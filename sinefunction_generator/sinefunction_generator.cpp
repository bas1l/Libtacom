#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <queue>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <random>
#include <atomic>
#include <sys/ioctl.h>
#include <string>
#include <ncurses.h>
#include <math.h>
#include <vector> 

#include "waveform.h"
#include "ad5383.h"
#include "utils.h"

#include "alphabet.h"
using namespace std;

#ifdef _WIN32
#include <windows.h>
#define SYSERROR()  GetLastError()
#else
#include <errno.h>
#define SYSERROR()  errno
#endif

    
    

uint16_t * create_sin(int freq, int ampl, int phase, int nsample, int offset)
{
	uint16_t * s;
	s = (uint16_t*) malloc(nsample * sizeof(uint16_t));
	
        // Suivant le nombre d'échantillons voulus :
	float incr = 2*M_PI/((float)nsample);
	for (int i=0; i<nsample; i++){
                //s[i] = sin(i*incr*freq +phase);
		s[i] = (uint16_t) floor(ampl * sin(i*incr*freq +phase) + offset);
                //printw("%i,", s[i]);
	}
	return s;
}

static int number_of_the_file = 1;
void write_file(std::vector<uint16_t> values, int freq, int ampl, int upto)
{
    std::string path = "caracterise2018/results/";
    std::string name =  path +
                        "n" + std::to_string(number_of_the_file) +
                        "_up" + std::to_string(upto) + 
                        "f" + std::to_string(freq) +
                        "amp" + std::to_string(ampl) +
                        "_theoric.csv";
    ofstream fichier(name, ios::out | ios::trunc);  //déclaration du flux et ouverture du fichier

    //cout << name << endl;
    if(fichier.is_open())  // si l'ouverture a réussi
    {
        // instructions
        for (int w=0; w<values.size(); ++w)
        {
            fichier << values[w];  // on l'affiche
            fichier << ", ";  // on l'affiche
        }
        fichier.close();  // on referme le fichier
    }
    else  // sinon
    {
        std::cerr<<"Failed to open file : "<<SYSERROR()<<std::endl;
        //cerr << "write_file/Erreur a l'ouverture !" << endl;
    }
    
    number_of_the_file++;
}

std::vector<uint16_t> push_sine_wave_ret(int freq, int ampl, int offset)
{
    int phase = 0;
    int nsample = 2000;
    std::vector<uint16_t> sinus;//nothing, just a break.
    
    uint16_t * s = create_sin(freq, ampl, phase, nsample, offset);
    
    for(int i=0; i < nsample; i++){
        sinus.push_back(s[i]);
    }
    //sinus.push_back(2048);
    
    return sinus;
}

std::vector<std::vector<uint16_t>> creatematrix(int nbsample, int value)
{
    std::vector<std::vector<uint16_t>> result(AD5383::num_channels);
    std::vector<uint16_t> onechan(nbsample, value);
    
    for(int c=0; c<AD5383::num_channels; c++)
    {
        result[c].assign(onechan.begin(), onechan.end());
    }
    
    return result;
}

void triple_spike(ALPHABET& alph, int chan_current, std::vector<std::vector<uint16_t>>& result)
{
    
    std::vector<std::vector<uint16_t>> spike = creatematrix(20, 0);
    std::vector<std::vector<uint16_t>> waitspike = creatematrix(20, 2048);
    std::vector<std::vector<uint16_t>> wait = creatematrix(2000, 2048);
    
    for (int i=0; i<3; i++)
    {
        for(int c=0; c<AD5383::num_channels; c++)
        {
            result[c].insert(result[c].end(),  wait[c].begin(), wait[c].end());
            if (c == chan_current)
            {
                result[c].insert(result[c].end(), spike[c].begin(), spike[c].end());
            }
            else
            {
                result[c].insert(result[c].end(), waitspike[c].begin(), waitspike[c].end());
            }
            
            
        }
    }
    for(int c=0; c<AD5383::num_channels; c++)
    {
        result[c].insert(result[c].end(),  wait[c].begin(), wait[c].end());
    }
    
}

void get_sinus(int f, int a, int u, int nos, int chan_current, ALPHABET& alph, std::vector<std::vector<uint16_t>>& result)
{
    
    std::vector<std::vector<uint16_t>> wait = creatematrix(2000, 2048);
    std::vector<std::vector<uint16_t>> sinus4all(AD5383::num_channels);
    
    
    std::vector<uint16_t> sinus = push_sine_wave_ret(f, a, u);
    std::vector<uint16_t> waitsinus(sinus.size(), 2048); //std::fill(waitsinus.begin(), waitsinus.end(), 2048);
    
    for(int chan=0; chan<AD5383::num_channels; chan++)
    {
        if (chan == chan_current)
        {
            sinus4all[chan].assign(sinus.begin(), sinus.end());
        }
        else
        {
            sinus4all[chan].assign(waitsinus.begin(), waitsinus.end());
        }
    }
    
    for (int i=0; i!=nos; i++)
    {
        for(int c=0; c<AD5383::num_channels; c++)
        {
            result[c].insert(result[c].end(), sinus4all[c].begin(), sinus4all[c].end());
            result[c].insert(result[c].end(),  wait[c].begin(), wait[c].end());   
        }
    }
    
}

static int amp_get_up = 500;
int get_up(std::vector<std::vector<uint16_t>>& result, int chan_used)
{
    int offset = 2048;
    int freq = 1;
    //int amp = 2000;
    
    std::vector<uint16_t> go_up = push_sine_wave_ret(freq, amp_get_up, offset);
    
    int go_up_length = (int)(go_up.size()/4);
    std::vector<uint16_t> waitsinus(go_up_length, 2048);
    
    for(int c=0; c<AD5383::num_channels; c++)
        {
            if (c == chan_used)
            {
                //result[c].push_back(1000);
                result[c].insert(result[c].end(), go_up.begin()+2*go_up_length, go_up.begin()+3*go_up_length);
            }
            else
            {
                //result[c].push_back(2048);
                result[c].insert(result[c].end(), waitsinus.begin(), waitsinus.end());
            }
        }

    
    
    printw("amp_get_up = %i\n", amp_get_up);
    return go_up[3*go_up_length];
}
    

void get_sinesweep(int fbeg, int fend, int amp1, int amp2, int up, int chan_used, int number_of_rep, std::vector<std::vector<uint16_t>>& result)
{
    //std::vector<uint16_t> sinus = push_sine_wave_ret(f, a, u);
    std::vector<uint16_t> waitsinus(2000, 2048);//sinus.size(), 2048); //std::fill(waitsinus.begin(), waitsinus.end(), 2048);
    
    int amp_used = amp1;
    int f = fbeg;
    
    if (fbeg<10)
    {
        for(f=fbeg; f<=5; f++)
        {
            for (int nor=0; nor<number_of_rep; nor++)
            {
                for(int c=0; c<AD5383::num_channels; c++)
                {
                    if (c == chan_used)
                    {
                        std::vector<uint16_t> sinus = push_sine_wave_ret(f, amp_used, up);
                        result[c].insert(result[c].end(), sinus.begin(), sinus.end());
                    }
                    else
                    {
                        result[c].insert(result[c].end(), waitsinus.begin(), waitsinus.end());
                    }
                }
            }
        }
        
        f=10;
    }
    
    for(; f<=fend; f+=5)
    {
        for (int nor=0; nor<number_of_rep; nor++)
        {
            for(int c=0; c<AD5383::num_channels; c++)
            {
                if (c == chan_used)
                {
                    std::vector<uint16_t> sinus = push_sine_wave_ret(f, amp_used, up);
                    result[c].insert(result[c].end(), sinus.begin(), sinus.end());

                }
                else
                {
                    result[c].insert(result[c].end(), waitsinus.begin(), waitsinus.end());
                }
            }
        }
    }

    // for amp2
    if (0)
    {
        int amp_used = amp2;
        for(int f=fbeg; f<=fend; f+=5)
        {
            for (int nor=0; nor<number_of_rep; nor++)
            {
                for(int c=0; c<AD5383::num_channels; c++)
                {
                    if (c == chan_used)
                    {
                        std::vector<uint16_t> sinus = push_sine_wave_ret(f, amp_used, up);
                        result[c].insert(result[c].end(), sinus.begin(), sinus.end());

                    }
                    else
                    {
                        result[c].insert(result[c].end(), waitsinus.begin(), waitsinus.end());
                    }
                }
            }
        }
    }
    
    
}

static int f_state = 1;
void getfrequencies(int *fbeg, int *fend)
{
    
    int fmin = 10;
    int fmax = 500;
    
    if (f_state == 1)
    {
        *fbeg = 1;
        *fend = 99;
    }
    else if (f_state == 2)
    {
        *fbeg = *fend+1;
        *fend = *fend+100;
    }
    else if (f_state == 3)
    {
        *fbeg = *fend+1;
        *fend = *fend+100;
    }
    else if (f_state == 4)
    {
        *fbeg = *fend+1;
        *fend = *fend+100;
    }
    else if (f_state == 5)
    {
        *fbeg = *fend+1;
        *fend = *fend+100;
    }
    
    printw("Frequencies statement = %i\n", f_state);
    
    f_state++;
}

std::vector<std::vector<uint16_t> > getvalues(char c, ALPHABET& alph)
{
    int chan_used = ACT_RINGFINGER2;
    
    static int f = 1;
    static int a = 1;
    static int u = 0;
    
    int fadd = 0;
    int aadd = 0;
    
    std::vector<std::vector<uint16_t> > result(AD5383::num_channels);
    
    
    switch (c)
    {
        // value to add to the current frequency
        case 'q':
        {
            fadd = -1;
            break;
        }
        case 'w':
        {
            fadd = +1;
            break;
        }
        case 'a':
        {
            fadd = -10;
            break;
        }
        case 's':
        {
            fadd = +10;
            break;
        }
        case 'z':
        {
            fadd = -100;
            break;
        }
        case 'x':
        {
            fadd = +100;
            break;
        }
        // value to add to the current amplitude
        case 'e':
        {
            aadd = -1;
            break;
        }
        case 'r':
        {
            aadd = +1;
            break;
        }
        case 'd':
        {
            aadd = -10;
            break;
        }
        case 'f':
        {
            aadd = +10;
            break;
        }
        case 'c':
        {
            aadd = -100;
            break;
        }
        case 'v':
        {
            aadd = +100;
            break;
        }
        // other type of movement
        case 'u':
        {// up statement
            u = get_up(result, chan_used);
            return result;
        }
        case 'n' :
        {// neutral statement
            u = 2048;
            for(int c=0; c<AD5383::num_channels; c++)
            {
                result[c].push_back(u);
                result[c].push_back(u);
            }
            return result;
        }
        default :
        {
            break;
        }
    }
    
    if ((f+fadd) <= 0)
    {
        printw("f<=0");
    }
    if ((a+aadd) <= 0)
    {
        printw("a<=0");
    }
    
    if (((f+fadd) > 0) && ((a+aadd) > 0))
    {
        f = f+fadd;
        a = a+aadd;
        
        get_sinus(f, a, u, 1, chan_used, alph, result);
    }
    
    printw("f=%i, ", f);
    printw("a=%i, ", a);
    printw("u=%i ::", u);
    
    printw("\n");
    return result;
}



int main(int argc, char *argv[])
{
    struct timespec t;
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
            perror("sched_setscheduler failed");
            exit(-1);
    }
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
    }
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    
    
    DEVICE dev;
    dev.configure();
    WAVEFORM wf;
    wf.configure();
    ALPHABET alph(dev, wf);//, AD5383::num_channels);
    alph.configure();
    // init drive electronics
    AD5383 ad;
    ad.spi_open();
    ad.configure();
    
    
    // fréquence maximale pour les sinus utilisées
    double hz_max = 1000; //Hz=1/s
    // th. de Nyquist implique :
    double freq_message = hz_max*2; // 2000 message / secondes (par chan)
    // un peu bizarre. Mais on souhaite faire 2 envoies de messages par millisec
    double timePmessage_ns = hz_max/freq_message * ms2ns; // * ns
    std::vector<std::vector<uint16_t> > values(AD5383::num_channels);
    std::vector<std::vector<uint16_t> > tmp(AD5383::num_channels);
    values = alph.getneutral();
    ad.execute_trajectory(values, timePmessage_ns);
    
    int ch;
    std::queue<char> letters;
    std::string str_used = "qwaszxerdfcvun";
    printw("You can start to write a letter, a word, a sentence \n --- When you are done, press '*' to Exit ---\n");
    do
    {    
        
        printw("l\n");
        if (ch != ERR)
        {
            printw("%c\n", ch);
            if (str_used.find(ch) != std::string::npos)
            {
                letters.push(ch);
            }
        }
        
        if (!letters.empty())
        {
            for (int w=0; w<values.size(); ++w)
            {
                values[w].clear();
            }
            values = getvalues(ch, alph);
            
            letters.pop();
        }
        else
        {
            ad.execute_trajectory(values, timePmessage_ns);
            printw("traj.");
        }
        
        printw("getch begin\n");
        ch = getchar();
        printw("getch end, %c\n", ch);

    }while(ch != '*');
    
    
    printw("\tWHC::End\n");
    refresh();
    endwin();
    
    
    return 0;
}
