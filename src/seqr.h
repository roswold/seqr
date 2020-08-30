#pragma once
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
#include"ui.h"
#include"wav.h"

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
#define MSG_NOP	0x00
#define MSG_OFF	0x80
#define MSG_ON	0x90

// Struct to define seqr internal state
typedef struct seqr_data
{
	// Synth data
	PaStream*pa;
	int32_t audio_data[512];
	uint32_t samplerate;
	uint32_t number_of_samples;
	uint32_t number_of_channels;
	uint32_t number_of_patterns;
	uint32_t notes_per_pattern;
	double volume;

	// Sequencer data
	//Msg seq[4][32];		// 4 Channels, 16 instructions/messages ('notes') per channel
	Msg*seq;
	char info[128];
	uint32_t patternoffset;

	// Misc
	int16_t *b;				// Buffer for samples
	pthread_t play_thread;	// Used to do output in another thread
} seqr_data;

// audio_thread_cb state
typedef struct audio_thread_cb_struct
{
	PaStream*pa;
	int16_t*b;
	unsigned int fc;
} audio_thread_cb_struct;

// Callback to play audio in seconday thread
void*seqr_audio_thread_cb(void*d);

// Exit program, free resources
//void quit(void);

// Exit on SIGINT
void sighandler(int sig);

// Helpful macros
//#define semitone					pow(2,1/12.0)
#define semitone					1.0594630943592953
#define raisesemi(freq,num_semi)	(freq*pow(semitone,(num_semi)))
#define midi2freq(key)				raisesemi(27.5,(key-21))

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
void seqr_close(seqr_data*seqr,ui_data*ui);

// Draw notes
void seqr_drawnotes(seqr_data*seqr,ui_data*ui);

// Draw UI
void seqr_drawui(seqr_data*seqr,ui_data*ui);

// Get, process keyboard input
void seqr_kb(seqr_data*seqr,ui_data*ui);

// Generate audio samples from internal sequence, synth data
void seqr_synthesize(seqr_data*seqr);

// Edit seqr data file
void seqr_edit_file(seqr_data*seqr,char*fn);

// Write seqr data file
void seqr_write_file(seqr_data*seqr,char*fn);

// Synthesize and play audio data
void seqr_play(seqr_data*seqr);

// Synthesize raw audio data and write to file
void seqr_write_raw_file(seqr_data*seqr,char*fn);

// Synthesize and write to WAV file along with WAV header
void seqr_export(seqr_data*seqr,char*fn);

// Get, process keyboard input
void seqr_kb(seqr_data*seqr,ui_data*ui);

// Convert MIDI number to human-readable note name
char*seqr_getnotename(int midi_key);

// Get currently selected note (Msg)
Msg*seqr_getcurmsg(seqr_data*seqr,ui_data*ui);

// Get note (Msg) at specified offset
Msg*seqr_getmsgat(seqr_data*seqr,int pat,int chan,int note);
