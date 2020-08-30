#include"wav.h"

WAVE wav_create(uint32_t samplerate,uint16_t bitspersample,uint16_t channels,uint32_t length)
{
	WAVE wav={
		.formatdatalength=16,
		.formattype=1,			// PCM
		.channels=channels,
		.samplerate=samplerate,
		.bittype=(uint16_t)(((uint32_t)bitspersample*channels)/8),
		.bitspersample=bitspersample,
		.datalength=length,
	};

	// Copy string data
	memcpy(wav.riff,"RIFF",4);
	memcpy(wav.wave,"WAVE",4);
	memcpy(wav.fmt,"fmt ",4);
	memcpy(wav.data,"data",4);
	return wav;
}
