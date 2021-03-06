#include <iostream>
#include <sys/mman.h>
#include "ad5383.h"
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
#include<ncurses.h>
#include <math.h>


using namespace std;

//counts every number that is added to the queue
static long long producer_count = 0;
//counts every number that is taken out of the queue
static long long consumer_count = 0;

static int ns2ms = 1000000;
static int hz22 = 1000; // ne doit jamais etre en dessous de 2.
static int ms_per_symbols = 1;


static const uint8_t a1 = 5;
static const uint8_t a2 = 4;
static const uint8_t a3 = 3;
static const uint8_t a4 = 17;
static const uint8_t a5 = 14;
static const uint8_t a6 = 22;
static const uint8_t  a7 = 12;
static const uint8_t  a8 = 9;
static const uint8_t  a9 = 15;
static const uint8_t  a10 = 2;
static const uint8_t  a11 = 21;
static const uint8_t  a12 = 18;
static const uint8_t  a13 = 19;
static const uint8_t  a14 = 13;
static const uint8_t  a15 = 8;
static const uint8_t  a16 = 16;
static const uint8_t  a17 = 1;
static const uint8_t  a18 = 20;
static const uint8_t  a19 = 7;
static const uint8_t  a20 = 23;
static const uint8_t  a21 = 10;
static const uint8_t  a22 = 7;
static const uint8_t  a23 = 6;
static const uint8_t  a24 = 0;


#define deg2rad 3.14159265/180

#define TAP_MOVE_DURATION 50 // ms

#define APPARENT_ASC_DURATION 500 // ms
#define APPARENT_MOVE_DURATION 250 // ms
#define APPARENT_DURATION   (APPARENT_ASC_DURATION+APPARENT_MOVE_DURATION)

#define APPARENT_MOVE_AMPLITUDE 700

float rand_a_b(int a, int b){
	//srand((unsigned) time(NULL));
	return (float)(rand() / (float)RAND_MAX) * (b-a) + a; 
}

float * create_envelope_sin(int length, int ampl){
	float * s;
	s = (float*) malloc(length * sizeof(float));
	
	float incr = M_PI/(float)length;
	 
	for (int i=0; i<length; i++){
		s[i] = ampl *( sin(i * incr)*0.5 + 0.5);
	}
	
	return s;
}

float * create_sin(int freq, int ampl, int phase, int nsample, int offset){
	float * s;
	s = (float*) malloc(nsample * sizeof(float));
	
	float incr = freq*2*M_PI/((float)nsample);
	 
	for (int i=0; i<nsample; i++){
		s[i] = ampl * sin(i*incr +phase) + offset;
	}
	
	return s;
}


float * create_envelope_asc(){
	float * s;
	s = (float*) malloc(APPARENT_ASC_DURATION * sizeof(float));
	
	float incr = M_PI/(float)(2*APPARENT_ASC_DURATION);
	 
	for (int i=0; i<APPARENT_ASC_DURATION; i++){
		s[i] = sin(i * incr);
	}
	
	return s;
}

float * create_random_dots(int length, int a, int b){
	float * r;
	r = (float*) malloc(length*sizeof(float));
	
	for (int t=0; t<length; t++){
		r[t] = rand_a_b(a, b);
	}
	
	return r;
}


std::vector<uint16_t> waveform_create_apparent(){
        std::vector<uint16_t> apparent_movement;
	int t;
	uint16_t inv = 4095;
	// ascension part 1/2
        
	float * asc = create_envelope_asc();
	
	for (t=0; t<APPARENT_ASC_DURATION; t++)
        {
		apparent_movement.push_back(inv - (uint16_t) (2000+1000*asc[t]));
                //printw("asc=%.f", asc[t]);
	}
	
	
	// pink noise part 2/2
	float * shape = create_envelope_sin(APPARENT_MOVE_DURATION, APPARENT_MOVE_AMPLITUDE);
	float * r = create_random_dots(APPARENT_MOVE_DURATION, -1, 1);
	
	for (t=0; t<APPARENT_MOVE_DURATION; t++){
		apparent_movement.push_back(inv - (uint16_t)(3000+ shape[t] * r[t]));
                printw("move=%f", shape[t] * r[t]);
	}
	
	
	free(asc);
	free(shape);
	free(r);
        
        return apparent_movement;
}



void push_channel(std::vector<std::vector<uint16_t> >& values, std::vector<int> channels){
    int ms_max = 50;
    int mid = ms_max/2;
    
    
    for(int j = 0; j < 31; ++j)
    {
        if (std::find(channels.begin(), channels.end(), j) != channels.end())
        {
            //printw("yolo\n");
            int ms;
            for(ms=0; ms<mid;ms++){
                values[j].push_back(0);
            }
            for(; ms<ms_max-1;ms++){
                values[j].push_back(4000);
            }
            values[j].push_back(2048);
            channels.erase(std::remove(channels.begin(), channels.end(), j), channels.end());
        }
        else
        {
            int ms;
            for(ms=0; ms<ms_max;ms++){
                values[j].push_back(2048);
            }
        }
    }
    
}


void letter_a_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a2};
    push_channel(values, channels);
}


void letter_b_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a8, a12, a13};
    push_channel(values, channels);
}


void letter_c_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a1, a2};
    push_channel(values, channels);
}


void letter_d_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a17, a24};
    push_channel(values, channels);
}


void letter_e_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a24};
    push_channel(values, channels);
}


void letter_f_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a17, a18};
    push_channel(values, channels);
}


void letter_g_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a11, a12, a13, a14, a15, a16, a17};
    push_channel(values, channels);
}


void letter_h_push(std::vector<std::vector<uint16_t> >& values, std::vector<uint16_t>& am){
    int number_act_used = 6;
    int size = am.size();
    int speed=12;
    int time_beforenext = size/speed;
    //int tbeforenext = size/6;;
    int total_time = (number_act_used+(speed-1))*time_beforenext+1;
    
}


void letter_i_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a23};
    push_channel(values, channels);
}


void letter_j_push(std::vector<std::vector<uint16_t> >& values, std::vector<uint16_t>& am){
    int number_act_used = 6;
    int size = am.size();
    int speed=12;
    int time_beforenext = size/speed;
    //int tbeforenext = size/6;;
    int total_time = (number_act_used+(speed-1))*time_beforenext+1;
    
    
    for(int j = 0; j < 31; ++j)
    {
        switch(j){
            case a23 :
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                for(int i=0; i<total_time-size;i++){
                    values[j].push_back(2048);
                }
                break;
                
            case a19 :
                for(int i=0; i<time_beforenext;i++){
                    values[j].push_back(2048);
                }
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                for(int i=0; i<total_time-size-time_beforenext;i++){
                    values[j].push_back(2048);
                }
                break;
                
            case a16 :
                for(int i=0; i<2*time_beforenext;i++){
                    values[j].push_back(2048);
                }
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                for(int i=0; i<total_time-size-2*time_beforenext;i++){
                    values[j].push_back(2048);
                }
                break;
                
            case a12 :
                for(int i=0; i<3*time_beforenext;i++){
                    values[j].push_back(2048);
                }
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                for(int i=0; i<total_time-size-3*time_beforenext;i++){
                    values[j].push_back(2048);
                }
                break;
            case a9 :
                for(int i=0; i<4*time_beforenext;i++){
                    values[j].push_back(2048);
                }
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                for(int i=0; i<total_time-size-4*time_beforenext;i++){
                    values[j].push_back(2048);
                }
                break;
                
            case a5 :
                for(int i=0; i<total_time-size-1;i++){
                    values[j].push_back(2048);
                }
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                values[j].push_back(2048);
                break;   
                
            default :
                for(int i=0; i<total_time;i++){
                    values[j].push_back(2048);
                }
                break;
        }
    }
}


void letter_k_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a16, a17, a18, a19 };
    push_channel(values, channels);
}


void letter_l_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a4, a5, a6 };
    push_channel(values, channels);
}


void letter_m_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a4, a5, a6, a8, a9, a10, a11, a12, a13 };
    push_channel(values, channels);
}


void letter_n_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a4, a5, a6, a8, a9, a10 };
    push_channel(values, channels);
}


void letter_o_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a22};
    push_channel(values, channels);
}


void letter_p_push(std::vector<std::vector<uint16_t> >& values, std::vector<uint16_t>& am){
    int number_act_used = 6;
    int size = am.size();
    int speed=12;
    int time_beforenext = size/speed;
    //int tbeforenext = size/6;;
    int total_time = (number_act_used+(speed-1))*time_beforenext+1;
    
    for(int j = 0; j < 31; ++j)
    {
        switch(j){
            case a24 :
                values[j].insert(std::end(values[j]), std::begin(am), std::end(am));
                values[j].push_back(2048);
                break;
                
            default :
                for(int i=0; i<total_time;i++){
                    values[j].push_back(2048);
                }
                break;
        }
    }
    //std::vector<int> channels {};
    //push_channel(values, channels);
}


void letter_q_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a10, a11 };
    push_channel(values, channels);
}


void letter_r_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a3, a9 };
    push_channel(values, channels);
}


void letter_s_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a14 };
    push_channel(values, channels);
}


void letter_t_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a7 };
    push_channel(values, channels);
}


void letter_u_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a21 };
    push_channel(values, channels);
}


void letter_v_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a4, a12 };
    push_channel(values, channels);
}


void letter_w_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a11, a17, a18, a24 };
    push_channel(values, channels);
}


void letter_x_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a15, a16, a17 };
    push_channel(values, channels);
}


void letter_y_push(std::vector<std::vector<uint16_t> >& values, std::vector<uint16_t>& am){
    //std::vector<int> channels { };
    //push_channel(values, channels);
}

void letter_z_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels {a7, a8};
    push_channel(values, channels);
}

void letter_space_push(std::vector<std::vector<uint16_t> >& values){
    std::vector<int> channels { 30 };//nothing, juste a break.
    push_channel(values, channels);
}

void push_letters(std::vector<std::vector<uint16_t> >& values, const char c, std::vector<uint16_t>& am, double * hz)
{
    switch(c){
        case 'a' :
            letter_a_push(values);
            break;
            
        case 'b' :
            letter_b_push(values);
            break;
            
        case 'c' :
            letter_c_push(values);
            break;
            
        case 'd' :
            letter_d_push(values);
            break;
            
        case 'e' :
            letter_e_push(values);
            break;
            
        case 'f' :
            letter_f_push(values);
            break;
            
        case 'g' :
            letter_g_push(values);
            break;
            
        case 'h' :
            letter_h_push(values, am);
            break;
            
        case 'i' :
            letter_i_push(values);
            break;
            
        case 'j' :
            *hz =0.5;
            letter_j_push(values, am);
            break;
            
        case 'k' :
            letter_k_push(values);
            break;
            
        case 'l' :
            letter_l_push(values);
            break;
            
        case 'm' :
            letter_m_push(values);
            break;
            
        case 'n' :
            letter_n_push(values);
            break;
            
        case 'o' :
            letter_o_push(values);
            break;
            
        case 'p' :
            letter_p_push(values, am);
            break;
            
        case 'q' :
            letter_q_push(values);
            break;
            
        case 'r' :
            letter_r_push(values);
            break;
            
        case 's' :
            letter_s_push(values);
            break;
            
        case 't' :
            letter_t_push(values);
            break;
            
        case 'u' :
            letter_u_push(values);
            break;
            
        case 'v' :
            letter_v_push(values);
            break;
            
        case 'w' :
            letter_w_push(values);
            break;
            
        case 'x' :
            letter_x_push(values);
            break;
            
        case 'y' :
            letter_y_push(values, am);
            break;
            
        case 'z' :
            letter_z_push(values);
            break;
            
        case ' ' :
            letter_space_push(values);
            break;
            
            
        default :
            
            break;
    }
    
}

void init_neutral(AD5383 ad, std::vector<std::vector<uint16_t> >& values)
{
    for(int j = 0; j < 31; ++j)
    {
        values[j].push_back(2048);
        values[j].push_back(2048);          
    }
    
    ad.execute_trajectory(values, 1000000);
}

void generateSentences(std::queue<char> & sentences, std::condition_variable & cv, std::mutex & m, std::atomic<bool> & workdone)
{
    int ch;

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    printw("WHC::create_sentences::Begin\n");

    printw("Press '*' to Exit :  \n");

    do{
        if (ch != ERR) {
            switch(ch){
                case KEY_UP: {
                    printw("\n\nUp Arrow\n");
                    printw("Press q to Exit\t");
                    break;
                }
                case KEY_DOWN:{
                    printw("\n\nDown Arrow\n");
                    printw("Press q to Exit\t");
                    break;
                }
                case KEY_LEFT:{
                    printw("\n\nLeft Arrow\n");
                    printw("Press q to Exit\t");
                    break;
                }
                case KEY_RIGHT:{
                    printw("\n\nRight Arrow\n");
                    printw("Press q to Exit\t");
                    break;
                }
                default:{
                    std::unique_lock<std::mutex> lk(m);
                    printw("%c", ch);
                    //printw("[GS][for] sentences.push(c)\n");
                    sentences.push(ch);
                    //printw("[GS][for] producer_count++()  %c\n", ch);

                    producer_count++;
                    //printw("[GS][for] cv.notify_one()\n");
                    cv.notify_one(); // Notify worker
                    //printw("[GS][for] cv.wait(lk)\n");
                    cv.wait(lk); // Wait for worker to complete
                    //printw("[GS][for] cv.wait(lk) :: FREE\n");
                    //printw("Press q to Exit\t");
                    break;
                }
            }
            // eoeowowej
          }
    }while((ch = getch()) != '*');
    
    
    printw("\tWHC::create_sentences::End\n");
    refresh();
    endwin();
    workdone = true;
    cv.notify_one(); // Notify worker
}

void workSymbols(std::queue<char> & sentences, std::condition_variable & cv, std::mutex & m, std::atomic<bool> & workdone)
{
    printw("\tWHC::workSymbols::Begin\n");
    // init drive electronics
    AD5383 ad;
    ad.spi_open();
    ad.configure();
    std::vector<std::vector<uint16_t> > values(AD5383::num_channels);
    printw("\tWHC::workSymbols::neutral\n");
    init_neutral(ad, values);
    
    std::vector<uint16_t> apparent_movement = waveform_create_apparent();
    std::queue<char> personalSentences;
    
    // Initialisation complete.
    //printw("\t{WS} notify_one()\n");
    cv.notify_one(); // Notify generator (placed here to avoid waiting for the lock)
    
    // The goal of this function is to use the letters putted by the other
    // thread, one by one, and play them consecutively.
    //printw("\t{WS} while...()\n");
    while(!workdone.load() or !sentences.empty())
    {
        //printw("{WS}{while} Begin\n");
        std::unique_lock<std::mutex> lk(m);
        
        //printw("{WS}{while} sentences.EMPTY ACHTOUNG()\n");
        if (sentences.empty())
        {
            //printw("\t{WS}{while} cv.wait(lk)\n");
            cv.wait(lk); // Wait for the generator to complete
            //printw("\t{WS}{while} cv.wait(lk) :: FREE\n");
        }
        //printw("\t{WS}{while} ..sentences.empty() :: 2\n");
        if (sentences.empty())
            continue;
        // free the common value asap
        //printw("{WS}{while} ..push moi ca !\n");
        personalSentences.push(sentences.front());
        //printw("\t{WS}{while} ..Catch symbol = %c\n", sentences.front());
        
        sentences.pop();
        consumer_count++;
        cv.notify_one(); // Notify generator (placed here to avoid waiting for the lock)
        
        // if last char is a space, then a word is finished
        if (personalSentences.front() != ' ')// is part of the alphabet){
        {
            double hz = ms_per_symbols;
            /*** move ACTUATORS ***/
            //printw("\tWHC::push_letters::begin [%c]\n", personalSentences.front());
            push_letters(values, personalSentences.front(), apparent_movement, &hz);
            //printw("\tWHC::execute_trajectory::begin [%c]\n", personalSentences.front());
            ad.execute_trajectory(values, hz*ns2ms);
            // for all channels, clear.
            for (int w=0; w<values.size(); ++w)
            {
                values[w].clear();
            }
            //std::vector<std::vector<uint16_t> > values(AD5383::num_channels);
            
                
            
           // printw("\tWHC::execute_trajectory::end [%c]\n", personalSentences.front());
            /*** move ACTUATORS ***/
        }
        personalSentences.pop();
     }
    
    printw("\tWS::workSymbols::End\n");
}


int main(int argc, char *argv[])
{
    std::cout << "WHC::Begin." << std::endl;

    struct timespec t;
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    /**************** C'EST AVEC CETTE OPTION QUE TOUT BUG *********************
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
            perror("sched_setscheduler failed");
            exit(-1);
    }
    ***************************************************************************/
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
    }

    
    
    // PARTIE THREADS
    std::condition_variable cv;
    std::mutex m;
    std::atomic<bool> workdone(false);
    std::queue<char> sentences;

    std::cout << "WHC::Threads" << std::endl;
    std::thread producer(generateSentences, std::ref(sentences), std::ref(cv), std::ref(m), std::ref(workdone));
    std::thread consumer(workSymbols, std::ref(sentences), std::ref(cv), std::ref(m), std::ref(workdone));
    
    //wait for 3 seconds, then join the threads
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    //workdone = true;
    //cv.notify_all(); // To prevent dead-lock

    producer.join();
    consumer.join();

    //output the counters
    std::cout << producer_count << std::endl;
    std::cout << consumer_count << std::endl;

    
    std::cout << "WHC::Done." << std::endl;

    //ad.spi_close();
  
    return 0;
}
