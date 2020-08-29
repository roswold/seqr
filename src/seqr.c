#include<stdio.h>
#include"seqr.h"
#include"ui.h"

void sighandler(int sig)
{
	//if(sig==SIGINT);
}

void*seqr_audio_thread_cb(void*d)
{
	//audio_thread_cb_struct s=*(audio_thread_cb_struct*)d;
	seqr_data*seqr=(seqr_data*)d;

	if(Pa_IsStreamStopped(seqr->pa))
	{
		Pa_StartStream(seqr->pa);
		Pa_WriteStream(seqr->pa,seqr->b,seqr->number_of_samples);
		Pa_StopStream(seqr->pa);
	}
	return NULL;
}

int16_t sine(double freq,double offset,double samplerate,double amplitude)
{
	return sin(((2.0L*3.14159265)/(samplerate/freq))*offset)*(amplitude/2.0L);
}

int16_t square(double freq,double offset,double samplerate,double amplitude)
{
	return (int16_t)fmod(offset,(samplerate/freq))<(samplerate/freq)/2?(amplitude/2.0L):-(amplitude/2.0L);
}

int16_t triangle(double freq,double offset,double samplerate,double amplitude)
{
	return ((amplitude*((samplerate/freq/2.0L)-(int16_t)fabs((int16_t)fmod((offset+(samplerate/freq/4.0L)),(2*(samplerate/freq/2.0L)))-(samplerate/freq/2.0L)) ))/(samplerate/freq/2.0L))-(amplitude/2.0L);
}

int16_t saw(double freq,double offset,double samplerate,double amplitude)
{
	return fmod((double)(offset+(int16_t)(samplerate/freq/2.0L)),((double)samplerate/freq))/((double)samplerate/freq)*(double)amplitude-(amplitude/2.0L);
}

int16_t noise(double freq,double offset,double samplerate,double amplitude)
{
	return (double)amplitude/2.0L-fmod(rand(),amplitude);
}

seqr_data*seqr_create(void)
{
	seqr_data*seqr;

	// seqr synth stuff
	seqr=malloc(sizeof(seqr_data));
	if(!seqr)
	{
		printf("failed to allocate seqr_data\n");
		return NULL;
	}

	srand(time(NULL));
	seqr->samplerate=44100;
	seqr->number_of_samples=44100;
	seqr->volume=11000;
	//seqr->bpm=120;
	seqr->notes_per_pattern=16;
	seqr->number_of_channels=4;

	// Allocate memory for sequence data (notes, channels, ...)
	seqr->seq=(Msg*)malloc(sizeof(Msg)*seqr->number_of_channels*seqr->notes_per_pattern);
	if(!seqr->seq)
	{
		printf("failed to allocate seqr_data sequence buffer\n");
		free(seqr);
		return NULL;
	}
	memset(seqr->seq,0,sizeof(Msg)*seqr->notes_per_pattern);

	// Allocate buffer for samples
	seqr->b=malloc(sizeof(int16_t)*seqr->number_of_samples);
	if(!seqr->b)
	{
		printf("failed to allocate seqr_data sample buffer\n");
		free(seqr);
		return NULL;
	}

	// Portaudio setup
	Pa_Initialize();
	Pa_OpenDefaultStream(&seqr->pa,0,1,paInt16,seqr->samplerate,seqr->number_of_samples,
		NULL,NULL);

	return seqr;
}

void seqr_synthesize(seqr_data*seqr)
{
	// Add each channel, divide by number of channels
	for(int i=0,j=0;i<seqr->notes_per_pattern;i++)
		for(int key=0;key<2000;key++)
			seqr->b[j]=	(sine(midi2freq(seqr->seq[0*seqr->notes_per_pattern+i].p1),j,seqr->samplerate,midi2freq(seqr->seq[0*seqr->notes_per_pattern+i].p1)?seqr->volume:0)+
						square(midi2freq(seqr->seq[1*seqr->notes_per_pattern+i].p1),j,seqr->samplerate,midi2freq(seqr->seq[1*seqr->notes_per_pattern+i].p1)?seqr->volume:0)+
						triangle(midi2freq(seqr->seq[2*seqr->notes_per_pattern+i].p1),j,seqr->samplerate,midi2freq(seqr->seq[2*seqr->notes_per_pattern+i].p1)?seqr->volume:0)+
						saw(midi2freq(seqr->seq[3*seqr->notes_per_pattern+i].p1),j,seqr->samplerate,midi2freq(seqr->seq[3*seqr->notes_per_pattern+i].p1)?seqr->volume:0)
						)/(double)seqr->number_of_channels,++j;
}

void seqr_drawnotes(seqr_data*seqr,ui_data*ui)
{
	//int ui_y=0,ui_channel=0;
	// Draw tracker note data/UI
	for(int i=0;i<seqr->notes_per_pattern;i++)
	{
		const int horiz_space=4;
		const int chan_width=16;
		int hilite_i=ui->note==i;

		attron(COLOR_PAIR(C_GREEN));
		mvprintw(i+1,1,"%0.2X",i);
		attroff(COLOR_PAIR(C_GREEN));

		for(int j=0;j<seqr->number_of_channels;++j)
		{
			int hilite=hilite_i&&ui->channel==j;
			int x_pos=j*chan_width;
			Msg*m=&seqr->seq[j*seqr->notes_per_pattern+i];

			// We need to sort of 'disassemble' the message queue here
			if(hilite)attron(COLOR_PAIR(C_HILITE));

			if(m->msg!=MSG_NOP)
			{
				if(!hilite)attron(COLOR_PAIR(C_WHITE));
				mvprintw(i+1,x_pos+=horiz_space,"%s",seqr_getnotename(m->p1));
				if(!hilite)attroff(COLOR_PAIR(C_WHITE));
			}
			else
			{
				mvprintw(i+1,x_pos+=horiz_space,"-");
			}

			if(!hilite)attron(COLOR_PAIR(C_BLUE));
			mvprintw(i+1,x_pos+=horiz_space,"%u",seqr->seq[j*seqr->notes_per_pattern+i].p1);
			if(!hilite)attroff(COLOR_PAIR(C_BLUE));

			if(!hilite)attron(COLOR_PAIR(C_MAGENTA));
			mvprintw(i+1,x_pos+=horiz_space,"%u\n",seqr->seq[j*seqr->notes_per_pattern+i].p2);
			if(!hilite)attroff(COLOR_PAIR(C_MAGENTA));

			if(hilite)attroff(COLOR_PAIR(C_HILITE));
		}
	}
}

void seqr_close(seqr_data*seqr,ui_data*ui)
{
	// Post message while we wait to close
	sprintf(seqr->info,"Closing audio");
	clear();
	seqr_drawui(seqr,ui);
	seqr_drawnotes(seqr,ui);
	refresh();

	// Join audio thread, free resources
	pthread_join(seqr->play_thread,NULL);
	if(seqr)
		Pa_StopStream(seqr->pa);
	Pa_Terminate();
	if(seqr->seq)
		free(seqr->seq);
	if(seqr->b)
		free(seqr->b);
	if(seqr)
		free(seqr);
}

void seqr_drawui(seqr_data*seqr,ui_data*ui)
{
	int y_pos=getmaxy(ui->w)-6;
	//setBackgroundColor(BLACK);
	attron(COLOR_PAIR(C_CYAN));
	char*chan_name[]={" (sine)"," (square)"," (triangle)"," (saw)"};
	mvprintw(y_pos++,1,"Channel:%u%s\n",ui->channel,chan_name[ui->channel]);
	attroff(COLOR_PAIR(C_CYAN));

	attron(COLOR_PAIR(C_WHITE));
	mvprintw(y_pos++,1,"_Q_ Quit\t_W_ Write\t_E_ Edit");
	mvprintw(y_pos++,1,"_X_ Export\t_R_ Export Raw\tSPACE Play");
	attroff(COLOR_PAIR(C_WHITE));

	// Draw info string if not empty
	if(strcmp(seqr->info,""))
	{
		attron(COLOR_PAIR(C_MAGENTA));
		mvprintw(++y_pos,1,"[%s]",seqr->info);
		attroff(COLOR_PAIR(C_MAGENTA));
	}
	box(ui->w,'|','-');
}

void seqr_edit_file(seqr_data*seqr,char*fn)
{
	FILE *f=fopen(fn,"rb");
	if(!f)
		sprintf(seqr->info,"Failed to open \"%s\"",fn);
	else
	{
		fread(seqr->seq,sizeof(Msg)*seqr->number_of_channels*seqr->notes_per_pattern,1,f);
		sprintf(seqr->info,"Opened \"%s\"",fn);
		fclose(f);
	}
}

void seqr_write_file(seqr_data*seqr,char*fn)
{
	FILE *f=fopen(fn,"wb");
	if(!f)return;
	fwrite(seqr->seq,sizeof(Msg)*seqr->number_of_channels*seqr->notes_per_pattern,1,f);
	sprintf(seqr->info,"Wrote \"%s\"",fn);
	fclose(f);
}

void seqr_play(seqr_data*seqr)
{
	seqr_synthesize(seqr);

	// Write synthesized output to stream
	//pthread_join(seqr->play_thread,NULL);
	pthread_create(&seqr->play_thread,NULL,seqr_audio_thread_cb,seqr);

	sprintf(seqr->info,"Playing stream");
}

void seqr_write_raw_file(seqr_data*seqr,char*fn)
{
	FILE *f=fopen("audio.dat","wb");
	if(!f)return;

	// Synthesize data
	seqr_synthesize(seqr);

	fwrite(seqr->b,sizeof(int16_t),seqr->number_of_samples,f);
	sprintf(seqr->info,"Wrote raw audio to \"audio.dat\"");
	fclose(f);
}

void seqr_export(seqr_data*seqr,char*fn)
{
	// Wave file header tailored for this purpose
	uint8_t hdr[]={0x52, 0x49, 0x46, 0x46, 0xac, 0x58, 0x01, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x44, 0xac, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00,
		0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x88, 0x58, 0x01, 0x00, 0x0a};

	// Generate, output samples
	FILE *f=fopen(fn,"wb");
	if(!f)return;

	// Synthesize data
	seqr_synthesize(seqr);

	fwrite(hdr,1,44,f);		// Write WAV header
	fwrite(seqr->b,sizeof(int16_t),seqr->number_of_samples,f);
	sprintf(seqr->info,"Exported \"%s\"",fn);
	fclose(f);
}

char*seqr_getnotename(int midi_key)
{
	static char name[8]={0};
	static char*base[]=
	{
		"A","A#","B","C",
		"C#","D","D#","E",
		"F","F#","G","G#",
	};

	sprintf(name,"%s%d",base[(midi_key-21)%12],((midi_key-21)/12)%128);
	return name;
}

void seqr_kb(seqr_data*seqr,ui_data*ui)
{
	int key=getch();
	int note=0;
	Msg*m;

	switch(key)
	{

		// Up
		case KEY_UP:
		case 'k':
			--ui->note;
			ui->note=(ui->note<0?seqr->notes_per_pattern-1:ui->note);
			break;

		// Down
		case KEY_DOWN:
		case 'j':
			ui->note=(ui->note+1)%seqr->notes_per_pattern;
			break;

		// Left
		case KEY_LEFT:
		case 'h':
			if(ui->channel==0)ui->channel=3;
			else --ui->channel;
			break;

		// Right
		case KEY_RIGHT:
		case 'l':
			ui->channel=(ui->channel+1)%4;
			break;

		// Quit
		case 'Q':
			ui->running=0;
			return;
			break;

		// Write file
		case 'W':
			seqr_write_file(seqr,"seq.dat");
			break;

		// Edit existing file
		case 'E':
			seqr_edit_file(seqr,"seq.dat");
			break;

		// Space --> Play audio
		case ' ':
			seqr_play(seqr);
			break;

		// Export raw data
		case 'R':
			seqr_write_raw_file(seqr,"audio.dat");
			break;

		// Export WAV file
		case 'X':
			seqr_export(seqr,"seqexport.wav");
			break;

		// ----- Piano keyboard layout for comp kb -----
		case 'z':
			note=48; //c3
			break;

		case 'x':
			note=50; //d3
			break;

		case 'c':
			note=52; //e3
			break;

		case 'v':
			note=53; //f3
			break;

		case 'b':
			note=55; //g3
			break;

		case 'n':
			note=57; //a3
			break;

		case 'm':
			note=59; //b3
			break;

		case 'q':
			note=60; //c4
			break;

		case 'w':
			note=62; //d4
			break;

		case 'e':
			note=64; //e4
			break;

		case 'r':
			note=65; //f4
			break;

		case 't':
			note=67; //g4
			break;

		case 'y':
			note=69; //a4
			break;

		case 'u':
			note=71; //b4
			break;

		case 'i':
			note=72; //c5
			break;

		case 'o':
			note=74; //d5
			break;

		case 'p':
			note=76; //e5
			break;

		// Clear note
		case 'a':
			m=&seqr->seq[ui->channel*seqr->notes_per_pattern+ui->note];
			m->msg=MSG_NOP;
			m->p1=0;
			break;

		default:
			break;
	}

	// Mark key as read
	key=-1;

	// Enter key into pattern
	if(note!=0)
	{
			m=&seqr->seq[ui->channel*seqr->notes_per_pattern+ui->note];
			m->msg=MSG_OFF;
			m->p1=note;
	}
}
