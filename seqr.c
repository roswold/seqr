#include<portaudio.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<math.h>
#include<time.h>
//#include<mmsystem.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#	include<pdcurses.h>
#else
#	include<ncurses.h>
#endif

void quit(void)
{
	Pa_Terminate();
	endwin();
}

void sighandler(int sig)
{
	if(sig==SIGINT)
		exit(1);
}

// Portaudio callback
int audio_cb(const void*inp,void*outp,long unsigned fc,
	PaStreamCallbackTimeInfo*ti,
	PaStreamCallbackFlags fl,void*data)
{
	inp=inp;
	ti=ti;
	fl=fl;
	int32_t*out=outp;
	int32_t*d=data;
	for(unsigned i=0;i<fc;++i)
		*out++=*d++;
	return 0;
}

// Audio is finished
void audio_finished_cb(void*data)
{
	int16_t*d=data;
	for(int i=0;i<44100;++i)
		d[i]=0;
}

#define SEMITC pow(2,1/12.0)
#define raisesemi(f,n) (f*pow(SEMITC,(n)))
int16_t sn(double f,double o,double r,double a)
{
	return sin(((2.0L*3.14159265)/(r/f))*o)*(a/2.0L);
}

int16_t sq(double f,double o,double r,double a)
{
	return (int16_t)fmod(o,(r/f))<(r/f)/2?(a/2.0L):-(a/2.0L);
}

int16_t tr(double f,double o,double r,double a)
{
	return ((a*((r/f/2.0L)-(int16_t)fabs((int16_t)fmod((o+(r/f/4.0L)),(2*(r/f/2.0L)))-(r/f/2.0L)) ))/(r/f/2.0L))-(a/2.0L);
}

int16_t sw(double f,double o,double r,double a)
{
	return fmod((double)(o+(int16_t)(r/f/2.0L)),((double)r/f))/((double)r/f)*(double)a-(a/2.0L);
}

int16_t ns(double f,double o,double r,double a)
{
	return (double)a/2.0L-fmod(rand(),a);
}


#define MSG_OFF	0x00
#define MSG_ON	0x01
typedef struct Msg
{
	uint32_t msg,p1,p2;
} Msg;

int main(int argc,char**argv)
{
	// Array of Msgs (16 per channel)
	Msg seq[4][16]={0};
	int key=-1;

	int32_t audio_data[512];
	PaStream*pa;

	// Parse command line options
	for(int i=0;i<argc;++i)
		if(strcmp(argv[i],"--help")==0 ||
			strcmp(argv[i],"-h")==0)
			puts("seqr version 0.0"),
			exit(0);

	// Signal hander, atexit
	signal(SIGINT,sighandler);
	atexit(quit);

	// ncurses setup
	initscr();
	start_color();

	// ncurses colors
	enum{C_CYAN=1,C_GREEN,C_YELLOW,C_WHITE,C_MAGENTA,C_BLUE,C_HILITE};
	init_pair(1,COLOR_CYAN,COLOR_BLACK);
	init_pair(2,COLOR_GREEN,COLOR_BLACK);
	init_pair(3,COLOR_YELLOW,COLOR_BLACK);
	init_pair(4,COLOR_WHITE,COLOR_BLACK);
	init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
	init_pair(6,COLOR_BLUE,COLOR_BLACK);
	init_pair(7,COLOR_BLACK,COLOR_WHITE);

	// seqr synth stuff
	srand(time(NULL));
	uint32_t samplerate=44100;
	uint32_t samples=samplerate;
	double fr=261;
	double a=11000;
	int32_t bpm=120;

	// Sequencer stuff
	// Msg seq[2][16];
	uint32_t y=0;
	char info[64]={0};
	uint8_t channel=0;
	uint32_t patternoffset=0;

	// Allocate buffer for samples
	int16_t *b=malloc(sizeof(int16_t)*samples);
	if(!b)
	{
		//gotoxy(1,1);
		mvprintw(1,1,"failed to load buffer\n");
		exit(1);
	}

	// Portaudio setup
	Pa_Initialize();
	Pa_OpenDefaultStream(&pa,0,1,paInt16,44100,44100/4,
		(PaStreamCallback*)audio_cb,b);

	// Tracker screen
	while(true)
	{


		// Draw tracker note data/UI
		for(int i=patternoffset;i<patternoffset+16;i++)
		{
			int yy=0;
			int yinc=8;
			int hilite=y==i;

			if(hilite)attron(COLOR_PAIR(C_HILITE));

			if(!hilite)attron(COLOR_PAIR(C_GREEN));
			mvprintw(i,yy,"%u",i);
			if(!hilite)attroff(COLOR_PAIR(C_GREEN));

			if(!hilite)attron(COLOR_PAIR(C_WHITE));
			mvprintw(i,yy+=yinc,"%u",seq[channel][i].msg);
			if(!hilite)attroff(COLOR_PAIR(C_WHITE));

			if(!hilite)attron(COLOR_PAIR(C_BLUE));
			mvprintw(i,yy+=yinc,"%u",seq[channel][i].p1);
			if(!hilite)attroff(COLOR_PAIR(C_BLUE));

			if(!hilite)attron(COLOR_PAIR(C_MAGENTA));
			mvprintw(i,yy+=yinc,"%u\n",seq[channel][i].p2);
			if(!hilite)attroff(COLOR_PAIR(C_MAGENTA));

			if(hilite)attroff(COLOR_PAIR(C_HILITE));
		}

		// Draw UI
		{
			int xx=17;
			//setBackgroundColor(BLACK);
			attron(COLOR_PAIR(C_CYAN));
			char*chan_name[]={" (sine)"," (square)"," (triangle)"," (saw)"};
			mvprintw(xx++,0,"Channel:%u%s\n",channel,chan_name[channel]);
			attroff(COLOR_PAIR(C_CYAN));

			attron(COLOR_PAIR(C_WHITE));
			mvprintw(xx++,0,"_Q_ Quit\t_W_ Write\t_E_ Edit");
			mvprintw(xx++,0,"_X_ Export\t_R_ Export Raw\tSPACE Play");
			attroff(COLOR_PAIR(C_WHITE));

			// Draw info string if not empty
			if(strcmp(info,""))
			{
				attron(COLOR_PAIR(C_MAGENTA));
				mvprintw(xx++,0,"\n[%s]",info);
				attroff(COLOR_PAIR(C_MAGENTA));
			}
		}

		// Check for keyboard input
		key=getch();
		if(key>0)
		{
			if(key=='Q')break;
			if(key==KEY_UP||key=='k')y=(y-patternoffset-1)%16+patternoffset;
			if(key==KEY_DOWN||key=='j')y=(y+1-patternoffset)%16+patternoffset;
			if(key==KEY_LEFT||key=='h')channel=--channel%4;
			if(key==KEY_RIGHT||key=='l')channel=++channel%4;
			if(key=='Q')exit(7);
			if(key=='W')
			{
				FILE *f=fopen("seq.dat","wb");
				if(!f)exit(7);
				fwrite(seq,1,sizeof(seq),f);
				//gotoxy(0,0);
				sprintf(info,"Wrote \"seq.dat\"");
				fclose(f);
			}
			if(key=='E')
			{
				FILE *f=fopen("seq.dat","rb");
				if(!f)exit(7);
				fread(seq,1,sizeof(seq),f);
				//gotoxy(0,0);
				sprintf(info,"Opened \"seq.dat\"");
				fclose(f);
			}

			// Space --> Play audio
			if(key==' ')
			{
				//HWAVEOUT hWaveOut = 0;
				//WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, samples, samples*2, 2, 16, 0 };
				//waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

				// Synthesize audio data
				for(int i=0,j=0;i<16;i++)
					for(int key=0;key<2000;key++)
						b[j]=(	sn(seq[0][i].msg,j,  samplerate,seq[0][i].msg?a:0)+
								sq(seq[1][i].msg,j,  samplerate,seq[1][i].msg?a:0)+
								tr(seq[2][i].msg,j,  samplerate,seq[2][i].msg?a:0)+
								sw(seq[3][i].msg,j++,samplerate,seq[3][i].msg?a:0)
								)/4.0;

				// Play audio
				//Pa_OpenDefaultStream(&pa,0,1,paInt16,44100,samples,
					//(PaStreamCallback*)audio_cb,b);
				//Pa_SetStreamFinishedCallback(pa,
					//(PaStreamFinishedCallback*)
						//audio_finished_cb);
				Pa_StartStream(pa);
				Pa_Sleep(1000);
				Pa_StopStream(pa);
				//Pa_CloseStream(pa);

				//WAVEHDR header = { b, samples, 0, 0, 0, 0, 0, 0 };
				//waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
				//waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
				//waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
				//waveOutClose(hWaveOut);
				// Sleep(samples);
				sprintf(info,"Played stream");
			}

			if(key=='R')//export raw data
			{
				FILE *f=fopen("audio.dat","wb");
				if(!f)exit(8);

				int samp=0;
				for(int i=0,j=0;i<16;i++)
					for(int key=0;key<2000;key++)
						b[j]=(	sn(seq[0][i].msg,j,  samplerate,seq[0][i].msg?a:0)+
								sq(seq[1][i].msg,j,  samplerate,seq[1][i].msg?a:0)+
								tr(seq[2][i].msg,j,  samplerate,seq[2][i].msg?a:0)+
								sw(seq[3][i].msg,j++,samplerate,seq[3][i].msg?a:0)
								)/4.0;


				fwrite(b,sizeof(int16_t),samples,f);
				sprintf(info,"Wrote raw audio to \"audio.dat\"");
				fclose(f);
			}
			if(key=='X')//export WAV file
			{
				//copy header
				uint8_t hdr[44];
				FILE *fi=fopen("wavhdr.dat","rb");
				if(!fi)return 9;
				fread(hdr,1,44,fi);
				fclose(fi);

				//generate, output samples
				FILE *f=fopen("seqexport.wav","wb");
				if(!f)exit(8);

				int samp=0;
				for(int i=0,j=0;i<16;i++)
					for(int key=0;key<2000;key++)
						b[j]=(	sn(seq[0][i].msg,j,  samplerate,seq[0][i].msg?a:0)+
								sq(seq[1][i].msg,j,  samplerate,seq[1][i].msg?a:0)+
								tr(seq[2][i].msg,j,  samplerate,seq[2][i].msg?a:0)+
								sw(seq[3][i].msg,j++,samplerate,seq[3][i].msg?a:0)
								)/4.0;


				fwrite(hdr,1,44,f);//write WAV header
				//fwrite(b,sizeof(int16_t),samples,f);
				fwrite(b,sizeof(int16_t),samples,f);
				//gotoxy(0,0);
				sprintf(info,"Exported \"seqexport.wav\"");
				fclose(f);
			}

			//piano keyboard layout for comp kb
			if(key>=97 && key<=122 &&
				(key!='h'&&key!='j'&&key!='k'&&key!='l'))
			{
				uint32_t fre=0;
				switch(key)
				{
				case 'z':
					fre=131;//c3
					break;
				case 'x':
					fre=147;//d3
					break;
				case 'c':
					fre=165;//e3
					break;
				case 'v':
					fre=175;//f3
					break;
				case 'b':
					fre=196;//g3
					break;
				case 'n':
					fre=220;//a3
					break;
				case 'm':
					fre=247;//b3
					break;
				// case ',':
				case 'q':
					fre=262;//c4
					break;
				case 'w':
					fre=294;//d4
					break;
				case 'e':
					fre=330;//e4
					break;
				case 'r':
					fre=349;//f4
					break;
				case 't':
					fre=392;//g4
					break;
				case 'y':
					fre=440;//a4
					break;
				case 'u':
					fre=494;//b4
					break;
				case 'i':
					fre=523;//c5
					break;
				case 'o':
					fre=587;//d5
					break;
				case 'p':
					fre=659;//e5
					break;
				default:
					break;
				}
				seq[channel][y].msg=fre;
			}

			// Mark key as read
			key=-1;
			// Clear screen in preparation for redraw
			clear();
		}

		// Update ncurses screen
		refresh();
		//usleep(20000);
	}
	// These called by atexit:
	//Pa_Terminate();
	//endwin();
}
