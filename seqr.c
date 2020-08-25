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
