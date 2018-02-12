
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <math.h>
#include <mutex>
#include <ncurses.h>
#include <queue>
#include <random>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/timerfd.h>

#include <time.h>
#include <thread>
#include <vector> 

#include "waveform.h"
#include "ad5383.h"
#include "utils.h"

#include "alphabet.h"
using namespace std;
using namespace std::chrono;

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







int execute(AD5383& ad, std::vector<std::vector<uint16_t> >& values, long period_ns)
{
    bool keep_running;
    int ret;
    unsigned long long missed = 0;
    int overruns = 0;
    int value_idx = 0;
    
    std::vector<uint16_t> values_target;
    int vmax = values[0].size();
    
    if(values.size() != AD5383::num_channels)
        throw std::runtime_error("Trajectory vector is bigger than number of channels");
    
    printw("PERIOD NANOSECONDS=%ld, ", period_ns);
    refresh();
    
    struct timespec ts = {
        .tv_sec = 0,
                .tv_nsec = period_ns
    };
    
    struct itimerspec its = {
        .it_interval = ts,
                .it_value = ts
    };
    
    int _timer_fd = timerfd_create(CLOCK_REALTIME, 0);
    if(_timer_fd == -1)
    {
        perror("execute_trajectory/timerfd_create");
        _timer_fd = 0;
        return overruns;
    }
    if(timerfd_settime(_timer_fd, 0, &its, NULL) == -1)
    {
        perror("execute_trajectory/timerfd_settime");
        close(_timer_fd);
        return overruns;
    }
    
    do
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        
        ret = read(_timer_fd, &missed, sizeof(missed));
        if (ret == -1)
        {
            perror("execute_trajectory/read");
            close(_timer_fd);
            return overruns;
        }
        overruns += missed - 1;
        
        keep_running = false;
        if(vmax > value_idx)
        {
            keep_running = true;
            values_target.clear();
            for(unsigned int channel = 0; channel < AD5383::num_channels; ++channel)
            {
                values_target.push_back(values[channel][value_idx]);
            }
            
            ad.execute_single_target(values_target);
        }
        ++value_idx;
        
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> dur = t2-t1;
        printw("duration=%ld, ", dur.count());
        refresh();
        
        
    } while(keep_running);

    
    
    close(_timer_fd);

    return overruns;
}










void read_letters(std::queue<char> & letters, std::mutex & mutexLetters, std::atomic<bool> & work)
{
    
    
    int ch = ERR;
    std::string str_used = "qwaszxerdfcvun";
    
    printw("---------------------------------------\n");
    printw("\tSine function generator\n");
    printw("---------------------------------------\n");
    
    printw("[Modify amplitude]\n");
    printw("\t+/-1   : 'q'=decrease, 'w'=increase\n");
    printw("\t+/-10  : 'a'=decrease, 's'=increase\n");
    printw("\t+/-100 : 'z'=decrease, 'x'=increase\n");
    
    printw("[Modify frequency]\n");
    printw("\t+/-1   : 'e'=decrease, 'r'=increase\n");
    printw("\t+/-10  : 'd'=decrease, 'f'=increase\n");
    printw("\t+/-100 : 'c'=decrease, 'v'=increase\n");
    
    printw("[Modify statement(offset)]\n");
    printw("\tUp      : 'u'\n");
    printw("\tNeutral : 'n'\n");
    
    printw("--- When you are done, press '*' to Exit ---\n");
    
    do
    {
        if (ch != ERR)
        {
            if (str_used.find(ch) != std::string::npos)
            {
                try
                {// using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
                    std::lock_guard<std::mutex> lk(mutexLetters);
                    letters.push(ch);
                }
                catch (std::logic_error&)
                {
                    std::cout << "[exception caught]\n";
                }
            }
            printw("%c", ch);
        }
    }while((ch = getch()) != '*');
    
    work = false;
}



void send_DAC(std::queue<char> & letters, std::mutex & mutexLetters, std::atomic<bool> & work)
{
    DEVICE dev;
    dev.configure();
    WAVEFORM wf;
    wf.configure();
    ALPHABET alph(dev, wf);//, AD5383::num_channels);
    alph.configure();
    AD5383 ad;
    ad.spi_open();
    ad.configure();
    
    int freq_message_per_sec = 2000; // message/s
    double dur_message_per_ms = (1/(double)freq_message_per_sec) *1000; // dur_message_per_sec * sec2ms
    long dur_message_per_ns = dur_message_per_ms * ms2ns; // * ns
    
    std::queue<char> letters_in;
    std::vector<std::vector<uint16_t> > values(AD5383::num_channels);
    values = alph.getneutral();
    
    ad.execute_trajectory(values, dur_message_per_ns);
    
    printw("dur_message_per_ns=%f", dur_message_per_ns);
    refresh();
    
    while(work.load())
    {
        try
        {// using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
            std::lock_guard<std::mutex> lk(mutexLetters);
            if (!letters.empty())
            {
                letters_in.push(letters.front());
                letters.pop();
            }
        }
        catch (std::logic_error&)
        {
            std::cout << "[exception caught]\n";
        }
        
        if (!letters_in.empty()) 
        {
            for (int w=0; w<values.size(); ++w)
                values[w].clear();
            values = getvalues(letters_in.front(), alph);
            letters_in.pop();
            printw("|");
            refresh();
        }
        else
        {
            execute(std::ref(ad), values, dur_message_per_ns);
            //printw(".");
            //refresh();
        }
    }
    
    
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

    // global variable
    std::queue<char> letters;
    std::mutex mutexLetters;
    std::atomic<bool> work(true);
    //std::condition_variable cv;
    
    std::thread thread_readLetters(read_letters, std::ref(letters), std::ref(mutexLetters), std::ref(work));
    std::thread thread_sendToDAC(send_DAC, std::ref(letters), std::ref(mutexLetters), std::ref(work));

    thread_sendToDAC.join();
    thread_readLetters.join();
    
    
    refresh();
    endwin();
    
    return 0;
}
