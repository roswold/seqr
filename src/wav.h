#pragma once
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

// Struct to contain .WAV file header data for exporting .WAV files
typedef struct WAVE
{
	int8_t riff[4];				// "RIFF" mark
	uint32_t filesize;			// File size
	int8_t wave[4];				// "WAVE" mark
	int8_t fmt[4];				// "fmt " mark
	uint32_t formatdatalength;	// Length of format data
	uint16_t formattype;		// Type (PCM for our purpose)
	uint16_t channels;			// Mono/stereo
	uint32_t samplerate;		// Samples per second
	uint32_t bitspersecond;		// Samplerate * Bits per sample
	uint16_t bittype;			// 1=>8bit mono, 2=>8bit stereo, 4=>16bit stereo...
	uint16_t bitspersample;		// e.g., 16, 32
	int8_t data[4];				// "data" mark
	uint32_t datalength;		// Length of audio
} WAVE;

// Create a custom WAVE header data struct for .WAV files
WAVE wav_create(uint32_t samplerate,uint16_t bitspersample,uint16_t channels,uint32_t length);
