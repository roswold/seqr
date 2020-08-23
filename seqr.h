#include<portaudio.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<math.h>
#include<pthread.h>
#include<time.h>
//#include<mmsystem.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#	include<pdcurses.h>
#else
#	include<ncurses.h>
#endif

// Struct to define sequential audio program messages
typedef struct Msg
{
	uint32_t msg,p1,p2;
} Msg;
#define MSG_OFF	0x00
#define MSG_ON	0x01

// Struct to define seqr internal state
typedef struct seqr_data
{
	// Synth data
	PaStream*pa;
	int32_t audio_data[512];
	uint32_t samplerate;
	uint32_t samples;
	double freq;
	double amplitude;
	int32_t bpm;

	// Sequencer data
	Msg seq[4][16];		// 4 Channels, 16 instructions/messages ('notes') per channel
	uint32_t y;
	char info[64];
	uint8_t channel;
	uint32_t patternoffset;

	// Allocate buffer for samples
	int16_t *b;
} seqr_data;

// audio_thread_cb state
typedef struct audio_thread_cb_struct
{
	PaStream*pa;
	int16_t*b;
	unsigned int fc;
} audio_thread_cb_struct;

// Callback to play audio in seconday thread
void*audio_thread_cb(void*d);

// Exit program, free resources
//void quit(void);

// Exit on SIGINT
void sighandler(int sig);

#define SEMITC pow(2,1/12.0)
#define raisesemi(f,n) (f*pow(SEMITC,(n)))

// Single-sample Sine wave oscillator
int16_t sine(double freq,double offset,double samplerate,double amplitude);

// Single-sample Square wave oscillator
int16_t square(double freq,double offset,double samplerate,double amplitude);

// Single-sample Triangle wave oscillator
int16_t triangle(double freq,double offset,double samplerate,double amplitude);

// Single-sample Sawtooth wave oscillator
int16_t saw(double freq,double offset,double samplerate,double amplitude);

// Single-sample Noise wave oscillator
int16_t noise(double freq,double offset,double samplerate,double amplitude);

// Allocate seqr_data, initialize
seqr_data*seqr_create(void);

// Free seqr resources
void seqr_close(seqr_data*seqr);

// Generate audio samples from internal sequence, synth data
void seqr_synthesize(seqr_data*seqr);
