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

    
    

uint16_t * create_sin(int freq, int ampl, int phase, int nsample, int offset){
	uint16_t * s;
	s = (uint16_t*) malloc(nsample * sizeof(uint16_t));
	
        // Suivant le nombre d'�chantillons voulus :
	float incr = 2*M_PI/((float)nsample);
	for (int i=0; i<nsample; i++){
                //s[i] = sin(i*incr*freq +phase);
		s[i] = (uint16_t) floor(ampl * sin(i*incr*freq +phase) + offset);
                //printw("%i,", s[i]);
	}
	return s;
}

void write_file(std::vector<uint16_t> values, int freq, int ampl, int upto)
{
    std::string path = "caracterise2018/results/";
    std::string name =  path +
                        "up" + std::to_string(upto) + 
                        "f" + std::to_string(freq) +
                        "amp" + std::to_string(ampl) +
                        "_theoric.csv";
    ofstream fichier(name, ios::out | ios::trunc);  //d�claration du flux et ouverture du fichier

    //cout << name << endl;
    if(fichier.is_open())  // si l'ouverture a r�ussi
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
    
    
}


std::vector<uint16_t> push_sine_wave_ret(int freq, int ampl, int offset){
    int phase = 0;
    int nsample = 2000;
    std::vector<uint16_t> sinus;//nothing, just a break.
    
    uint16_t * s = create_sin(freq, ampl, phase, nsample-1, offset);
    
    for(int i=0; i < nsample-1; i++){
        sinus.push_back(s[i]);
    }
    sinus.push_back(2048);
    
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

void triple_spike(ALPHABET& alph, std::vector<std::vector<uint16_t>>& result)
{
    
    std::vector<std::vector<uint16_t>> spike = creatematrix(20, 1500);
    std::vector<std::vector<uint16_t>> wait = creatematrix(2000, 2048);
    
    for (int i=0; i<3; i++)
    {
        for(int c=0; c<AD5383::num_channels; c++)
        {
            result[c].insert(result[c].end(),  wait[c].begin(), wait[c].end());
            result[c].insert(result[c].end(), spike[c].begin(), spike[c].end());
        }
    }
    for(int c=0; c<AD5383::num_channels; c++)
    {
        result[c].insert(result[c].end(),  wait[c].begin(), wait[c].end());
    }
    
}

void get_sinus(int f, int a, int u, int nos, ALPHABET& alph, std::vector<std::vector<uint16_t>>& result)
{
    
    std::vector<std::vector<uint16_t>> wait = creatematrix(2000, 2048);
    std::vector<std::vector<uint16_t>> sinus4all(AD5383::num_channels);
    
    
    std::vector<uint16_t> sinus = push_sine_wave_ret(f, a, u);
    std::vector<uint16_t> waitsinus(sinus.size(), 2048); //std::fill(waitsinus.begin(), waitsinus.end(), 2048);
    
    for(int chan=0; chan<AD5383::num_channels; chan++)
    {
        if (chan == 11)
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

int get_up(std::vector<std::vector<uint16_t>>& result)
{
    int offset = 2048;
    int freq = 1;
    int amp = 2000;
    
    std::vector<uint16_t> go_up = push_sine_wave_ret(freq, amp, offset);
    
    int go_up_length = (int)(go_up.size()/4);
    std::vector<uint16_t> waitsinus(go_up_length, 2048);
    
    int chan_used = 11;
    for(int c=0; c<AD5383::num_channels; c++)
        {
            if (c == chan_used)
            {
                result[c].insert(result[c].end(), go_up.begin()+go_up_length, go_up.end());
            }
            else
            {
                result[c].insert(result[c].end(), waitsinus.begin(), waitsinus.end());
            }
        }

    
    
    return go_up[go_up_length];
}
    
    
void get_sinesweep(int fbeg, int fend, int amp1, int amp2, int up, std::vector<std::vector<uint16_t>>& result)
{
    //std::vector<uint16_t> sinus = push_sine_wave_ret(f, a, u);
    std::vector<uint16_t> waitsinus(2000, 2048);//sinus.size(), 2048); //std::fill(waitsinus.begin(), waitsinus.end(), 2048);
    
    int chan_used = 11;
    
    for(int f=fbeg; f<=fend; f++)
    {
        for(int c=0; c<AD5383::num_channels; c++)
        {
            if (c == chan_used)
            {
                std::vector<uint16_t> sinus = push_sine_wave_ret(f, amp1, up);
                result[c].insert(result[c].end(), sinus.begin(), sinus.end());
                
            }
            else
            {
                result[c].insert(result[c].end(), waitsinus.begin(), waitsinus.end());
            }
        }
    }
    
    // for amp2
    if (0)
    {
        for(int f=fbeg; f<=fend; f++)
        {
            for(int c=0; c<AD5383::num_channels; c++)
            {
                if (c == chan_used)
                {
                    std::vector<uint16_t> sinus = push_sine_wave_ret(f, amp2, up);
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


std::vector<std::vector<uint16_t> > getvalues(char c, ALPHABET& alph)
{
    static int a = 0;
    static int u = 0;
    static int f = 0;
    
    static int freq[] = {1, 2, 5, 10, 20, 50, 100, 200, 300, 500, 700};
    static int freq_max = sizeof(freq)/sizeof(int);

    static int ampl[] = {50, 100, 150, 200, 250, 300, 400, 600, 800, 1000};
    static int ampl_max = sizeof(ampl)/sizeof(int);

    static int upto[] = {2048};
    //static int upto_max = sizeof(upto)/sizeof(int);
    
    std::vector<std::vector<uint16_t> > result(AD5383::num_channels);
    static int up = 2048;
    
    switch (c)
    {
        case 'k' :
        {
            int ff = freq[f];
            int aa = ampl[a];
            int uu = upto[u];
            int num_of_sinus = 2;
            
            triple_spike(alph, result); // to fit between the laser data and the theoric data
            get_sinus(ff, aa, uu, num_of_sinus, alph, result);
            
            std::vector<uint16_t> towrite(result[11].begin(), result[11].end());
            write_file(towrite, ff, aa, uu);
            
            printw("::After : go. \n");
            break;
        }
        case 'a' :     
        {
            if (a == ampl_max-1)
            {
                if (f == freq_max-1)
                {
                    printw("::After(f=%i,a=%i)/Impossible\t", freq[f], ampl[a]);
                }
                else
                {
                    f += 1;
                    a = 0;
                    printw("::After(f=%i,a=%i).", freq[f], ampl[a]);
                }
            }
            else
            {
                a += 1;
                printw("::After(f=%i,a=%i).", freq[f], ampl[a]);
            }
            
            
        break;
        }
        case 'b' :
        {   
            if (a == 0)
            {
                if (f == 0)
                {
                    printw("::After(f=%i,a=%i)/Impossible\t", freq[f], ampl[a]);
                }
                else
                {
                    f -= 1;
                    a = ampl_max-1;
                    printw("::After(f=%i,a=%i).", freq[f], ampl[a]);
                }
                
            }
            else
            {
                a -= 1;
                printw("::After(f=%i,a=%i).", freq[f], ampl[a]);
            }
            
        break;
        }
        
        case 'u':
        {
            up = get_up(result);
            break;
        }
        case 'f':
        {
            int fbeg = 10;
            int fend = 500;
            int amp1 = 50;
            int amp2 = 500;
            
            get_sinesweep(fbeg, fend, amp1, amp2, up, result);
            break;
        }
        
        default :
            
        break;
    }

    
    return result;
}


void generateSentences(std::queue<char> & sentences, std::condition_variable & cv,
            std::mutex & m, std::atomic<bool> & workdone, std::string str_alph)
{

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    
    std::string str_ponc = " .,;:!?-";
    
    int ch;
    printw("You can start to write a letter, a word, a sentence \n --- When you are done, press '*' to Exit ---\n");
    do{
        if (ch != ERR) 
        {
            // if part of the alphabet
            if (str_alph.find(ch) != std::string::npos)
            {
                printw("%c", ch);
                
                std::unique_lock<std::mutex> lk(m);
                sentences.push(ch);
                
                cv.notify_one();
                cv.wait(lk);
            }// if part of the ponctuation
            else if (str_ponc.find(ch) != std::string::npos)
            {
                printw("%c", ch);
            }
            else
            {
                printw("\n<Key not implemented> Need to Exit ? Press '*'.\n");
            }
          }
    }while((ch = getch()) != '*');
    
    
    printw("\tWHC::create_sentences::End\n");
    refresh();
    endwin();
    workdone = true;
    cv.notify_one(); // Notify worker
}



void workSymbols(std::queue<char> & sentences, std::condition_variable & cv, 
                std::mutex & m, std::atomic<bool> & workdone, ALPHABET& alph)
{
    // init drive electronics
    AD5383 ad;
    ad.spi_open();
    ad.configure();
    // fr�quence maximale pour les sinus utilis�es
    double hz_max = 1000; //Hz=1/s
    // th. de Nyquist implique :
    double freq_message = hz_max*2; // 2000 message / secondes (par chan)
    // un peu bizarre. Mais on souhaite faire 2 envoies de messages par millisec
    double timePmessage_ns = hz_max/freq_message * ms2ns; // * ns
    
    std::vector<std::vector<uint16_t> > values(AD5383::num_channels);
    values = alph.getneutral();
    ad.execute_trajectory(values, timePmessage_ns);
    for (int w=0; w<values.size(); ++w)
    {
        values[w].clear();
    }
    
    std::queue<char> letters;
    
    // Initialisation complete.
    cv.notify_one(); // Notify generator (placed here to avoid waiting for the lock)
    
    // The goal of this function is to use the letters putted by the other
    // thread, one by one, and play them consecutively.
    while(!workdone.load() or !sentences.empty())
    {
        std::unique_lock<std::mutex> lk(m);

        if (sentences.empty())
        {
            cv.wait(lk); // Wait for the generator to complete
        }
        if (sentences.empty()) 
            continue;
        
        // free the common value asap
        letters.push(sentences.front());
        sentences.pop();
        cv.notify_one(); // Notify generator (placed here to avoid waiting for the lock)
        
        // if last char is a space, then a word is finished
        if (letters.front() != ' ')// is part of the alphabet){
        {
            //values = alph.getl(letters.front());
            values = getvalues(letters.front(), alph);
            ad.execute_trajectory(values, timePmessage_ns);
            
            // for all channels, clear.
            for (int w=0; w<values.size(); ++w)
            {
                values[w].clear();
            }
        }
        letters.pop();
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
    
    DEVICE dev;
    dev.configure();
    WAVEFORM wf;
    wf.configure();
    ALPHABET alph(dev, wf);//, AD5383::num_channels);
    alph.configure();
    
    std::condition_variable cv;
    std::mutex m;
    std::atomic<bool> workdone(false);
    std::queue<char> sentences;
    
    
    std::thread extract_text;
    extract_text = std::thread(workSymbols, std::ref(sentences), std::ref(cv), 
                           std::ref(m), std::ref(workdone), std::ref(alph));
    std::thread send_to_dac;
    send_to_dac = std::thread(generateSentences, std::ref(sentences), std::ref(cv),
                           std::ref(m), std::ref(workdone), alph.getlist_alphabet());
    
    extract_text.join();
    send_to_dac.join();
    
    
    
    return 0;
}