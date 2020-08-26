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
	seqr->number_of_samples=seqr->samplerate;
	seqr->volume=11000;
	//seqr->bpm=120;
	seqr->notes_per_pattern=16;
	memset(seqr->seq,0,sizeof(Msg)*2*seqr->notes_per_pattern);

	// Allocate buffer for samples
	seqr->b=malloc(sizeof(int16_t)*seqr->number_of_samples);
	if(!seqr->b)
	{
		printf("failed to allocate seqr_data buffer\n");
		free(seqr);
		return NULL;
	}
	return seqr;
}

void seqr_synthesize(seqr_data*seqr)
{
	for(int i=0,j=0;i<seqr->notes_per_pattern;i++)
		for(int key=0;key<2000;key++)
			seqr->b[j]=	(sine(seqr->seq[0][i].msg,j,seqr->samplerate,seqr->seq[0][i].msg?seqr->volume:0)+
						square(seqr->seq[1][i].msg,j,seqr->samplerate,seqr->seq[1][i].msg?seqr->volume:0)+
						triangle(seqr->seq[2][i].msg,j,seqr->samplerate,seqr->seq[2][i].msg?seqr->volume:0)+
						saw(seqr->seq[3][i].msg,j,seqr->samplerate,seqr->seq[3][i].msg?seqr->volume:0)
						)/4.0,++j;
}

void seqr_drawnotes(seqr_data*seqr,ui_data*ui)
{
	//int ui_y=0,ui_channel=0;
	// Draw tracker note data/UI
	for(int i=0;i<seqr->notes_per_pattern;i++)
	{
		const int vert_space=8;
		int y_pos=0;
		int hilite=ui->note==i;

		if(hilite)attron(COLOR_PAIR(C_HILITE));

		if(!hilite)attron(COLOR_PAIR(C_GREEN));
		mvprintw(i,y_pos,"%u",i);
		if(!hilite)attroff(COLOR_PAIR(C_GREEN));

		if(!hilite)attron(COLOR_PAIR(C_WHITE));
		mvprintw(i,y_pos+=vert_space,"%u",seqr->seq[ui->channel][i].msg);
		if(!hilite)attroff(COLOR_PAIR(C_WHITE));

		if(!hilite)attron(COLOR_PAIR(C_BLUE));
		mvprintw(i,y_pos+=vert_space,"%u",seqr->seq[ui->channel][i].p1);
		if(!hilite)attroff(COLOR_PAIR(C_BLUE));

		if(!hilite)attron(COLOR_PAIR(C_MAGENTA));
		mvprintw(i,y_pos+=vert_space,"%u\n",seqr->seq[ui->channel][i].p2);
		if(!hilite)attroff(COLOR_PAIR(C_MAGENTA));

		if(hilite)attroff(COLOR_PAIR(C_HILITE));
	}
}

void seqr_close(seqr_data*seqr)
{
	if(seqr)
		Pa_StopStream(seqr->pa);
	Pa_Terminate();
	if(seqr->b)
		free(seqr->b);
	if(seqr)
		free(seqr);
}

void seqr_drawui(seqr_data*seqr,ui_data*ui)
{
	int x_pos=17;
	//setBackgroundColor(BLACK);
	attron(COLOR_PAIR(C_CYAN));
	char*chan_name[]={" (sine)"," (square)"," (triangle)"," (saw)"};
	mvprintw(x_pos++,0,"Channel:%u%s\n",ui->channel,chan_name[ui->channel]);
	attroff(COLOR_PAIR(C_CYAN));

	attron(COLOR_PAIR(C_WHITE));
	mvprintw(x_pos++,0,"_Q_ Quit\t_W_ Write\t_E_ Edit");
	mvprintw(x_pos++,0,"_X_ Export\t_R_ Export Raw\tSPACE Play");
	attroff(COLOR_PAIR(C_WHITE));

	// Draw info string if not empty
	if(strcmp(seqr->info,""))
	{
		attron(COLOR_PAIR(C_MAGENTA));
		mvprintw(x_pos++,0,"\n[%s]",seqr->info);
		attroff(COLOR_PAIR(C_MAGENTA));
	}
}

void seqr_edit_file(seqr_data*seqr,char*fn)
{
	FILE *f=fopen(fn,"rb");
	if(!f)
		sprintf(seqr->info,"Failed to open \"%s\"",fn);
	else
	{
		fread(seqr->seq,sizeof(seqr->seq),1,f);
		sprintf(seqr->info,"Opened \"%s\"",fn);
		fclose(f);
	}
}

void seqr_write_file(seqr_data*seqr,char*fn)
{
	FILE *f=fopen(fn,"wb");
	if(!f)return;
	fwrite(seqr->seq,sizeof(seqr->seq),1,f);
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

void seqr_kb(seqr_data*seqr,ui_data*ui)
{
	int key=getch();
	uint32_t freq=0;

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

		// Piano keyboard layout for comp kb
		case 'z':
			freq=131;//c3
			break;

		case 'x':
			freq=147;//d3
			break;

		case 'c':
			freq=165;//e3
			break;

		case 'v':
			freq=175;//f3
			break;

		case 'b':
			freq=196;//g3
			break;

		case 'n':
			freq=220;//a3
			break;

		case 'm':
			freq=247;//b3
			break;

		case 'q':
			freq=262;//c4
			break;

		case 'w':
			freq=294;//d4
			break;

		case 'e':
			freq=330;//e4
			break;

		case 'r':
			freq=349;//f4
			break;

		case 't':
			freq=392;//g4
			break;

		case 'y':
			freq=440;//a4
			break;

		case 'u':
			freq=494;//b4
			break;

		case 'i':
			freq=523;//c5
			break;

		case 'o':
			freq=587;//d5
			break;

		case 'p':
			freq=659;//e5
			break;

		default:
			break;
	}

	// Mark key as read
	key=-1;
	if(freq!=0)
		seqr->seq[ui->channel][ui->note].msg=freq;
}
