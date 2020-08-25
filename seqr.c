#include<stdio.h>
#include"seqr.h"
#include"ui.h"

// Entry
int main(int argc,char**argv)
{
	ui_data*ui;
	seqr_data*seqr;

	// Parse command line options
	for(int i=0;i<argc;++i)
		if(strcmp(argv[i],"--help")==0 || strcmp(argv[i],"-h")==0)
			puts("seqr version 0.0 [--help|-h]");

	// Initialize state
	ui=ui_create();
	seqr=seqr_create();
	signal(SIGINT,sighandler);

	// Portaudio setup
	Pa_Initialize();
	Pa_OpenDefaultStream(&seqr->pa,0,1,paInt16,44100,seqr->number_of_samples,
		NULL,NULL);

	// Main loop
	//int ui_y=0;
	//int ui_channel=0;
	while(ui->running)
	{

		// Draw UI
		clear();
		seqr_drawnotes(seqr,ui);
		seqr_drawui(seqr,ui);

		// Check for keyboard input
		//seqr_kb(seqr,ui);
		{
			if(!ui->running)break;
			int key=getch();
			if(key>0)
			{
				if(key=='Q')break;
				if(key==KEY_UP||key=='k')
					--ui->note,
					ui->note=(ui->note<0?seqr->notes_per_pattern-1:ui->note);
				if(key==KEY_DOWN||key=='j')ui->note=(ui->note+1)%seqr->notes_per_pattern;
				if(key==KEY_LEFT||key=='h')
				{
					if(ui->channel==0)ui->channel=3;
					else --ui->channel;
				}
				if(key==KEY_RIGHT||key=='l')ui->channel=(ui->channel+1)%4;
				if(key=='Q')
				{
					ui->running=0;
					break;
				}

				// Write file
				if(key=='W')
				{
					FILE *f=fopen("seq.dat","wb");
					if(!f)break;
					fwrite(seqr->seq,sizeof(seqr->seq),1,f);
					//gotoxy(0,0);
					sprintf(seqr->info,"Wrote \"seq.dat\"");
					fclose(f);
				}

				// Edit existing file
				if(key=='E')
				{
					FILE *f=fopen("seq.dat","rb");
					if(!f)
						sprintf(seqr->info,"Failed to open \"seq.dat\"");
					else
					{
						fread(seqr->seq,sizeof(seqr->seq),1,f);
						//gotoxy(0,0);
						sprintf(seqr->info,"Opened \"seq.dat\"");
						fclose(f);
					}
				}

				// Space --> Play audio
				if(key==' ')
				{

					seqr_synthesize(seqr);

					// Write synthesized output to stream
					//pthread_join(seqr->play_thread,NULL);
					audio_thread_cb_struct s={.pa=seqr->pa,.b=seqr->b,.fc=(seqr->number_of_samples)};
					pthread_create(&seqr->play_thread,NULL,audio_thread_cb,&s);

					sprintf(seqr->info,"Playing stream");
				}

				// Export raw data
				if(key=='R')
				{
					FILE *f=fopen("audio.dat","wb");
					if(!f)break;

					// Synthesize data
					seqr_synthesize(seqr);

					fwrite(seqr->b,sizeof(int16_t),seqr->number_of_samples,f);
					sprintf(seqr->info,"Wrote raw audio to \"audio.dat\"");
					fclose(f);
				}

				// Export WAV file
				if(key=='X')
				{
					// Wave file header tailored for this purpose
					uint8_t hdr[]={0x52, 0x49, 0x46, 0x46, 0xac, 0x58, 0x01, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
						0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x44, 0xac, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00,
						0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x88, 0x58, 0x01, 0x00, 0x0a};

					// Generate, output samples
					FILE *f=fopen("seqexport.wav","wb");
					if(!f)break;

					// Synthesize data
					seqr_synthesize(seqr);

					fwrite(hdr,1,44,f);		// Write WAV header
					fwrite(seqr->b,sizeof(int16_t),seqr->number_of_samples,f);
					sprintf(seqr->info,"Exported \"seqexport.wav\"");
					fclose(f);
				}

				//piano keyboard layout for comp kb
				if(key>=97 && key<=122 &&
					(key!='h'&&key!='j'&&key!='k'&&key!='l'))
				{
					uint32_t freq=0;
					switch(key)
					{
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
					// case ',':
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
					seqr->seq[ui->channel][ui->note].msg=freq;
				}

				// Mark key as read
				key=-1;
				// Clear screen in preparation for redraw
			}
		}

		// Update ncurses screen
		refresh();
	}

quit:
	seqr_close(seqr);
	ui_close(ui);
}

void sighandler(int sig)
{
	//if(sig==SIGINT);
}

void*audio_thread_cb(void*d)
{
	audio_thread_cb_struct*s=(audio_thread_cb_struct*)d;

	if(Pa_IsStreamStopped(s->pa))
	{
		Pa_StartStream(s->pa);
		Pa_WriteStream(s->pa,s->b,s->fc);
		Pa_StopStream(s->pa);
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

void seqr_kb(seqr_data*seqr,ui_data*ui)
{
	if(!ui->running)return;
	int key=getch();
	if(key>0)
	{
		if(key=='Q')return;
		if(key==KEY_UP||key=='k')
			--ui->note,
			ui->note=(ui->note<0?seqr->notes_per_pattern-1:ui->note);
		if(key==KEY_DOWN||key=='j')ui->note=(ui->note+1)%seqr->notes_per_pattern;
		if(key==KEY_LEFT||key=='h')
		{
			if(ui->channel==0)ui->channel=3;
			else --ui->channel;
		}
		if(key==KEY_RIGHT||key=='l')ui->channel=(ui->channel+1)%4;
		if(key=='Q')
		{
			ui->running=0;
			return;
		}

		// Write file
		if(key=='W')
		{
			FILE *f=fopen("seq.dat","wb");
			if(!f)return;
			fwrite(seqr->seq,sizeof(seqr->seq),1,f);
			//gotoxy(0,0);
			sprintf(seqr->info,"Wrote \"seq.dat\"");
			fclose(f);
		}

		// Edit existing file
		if(key=='E')
		{
			FILE *f=fopen("seq.dat","rb");
			if(!f)
				sprintf(seqr->info,"Failed to open \"seq.dat\"");
			else
			{
				fread(seqr->seq,sizeof(seqr->seq),1,f);
				//gotoxy(0,0);
				sprintf(seqr->info,"Opened \"seq.dat\"");
				fclose(f);
			}
		}

		// Space --> Play audio
		if(key==' ')
		{

			seqr_synthesize(seqr);

			// Write synthesized output to stream
			//pthread_join(seqr->play_thread,NULL);
			audio_thread_cb_struct s={.pa=seqr->pa,.b=seqr->b,.fc=(seqr->number_of_samples)};
			pthread_create(&seqr->play_thread,NULL,audio_thread_cb,&s);

			sprintf(seqr->info,"Playing stream");
		}

		// Export raw data
		if(key=='R')
		{
			FILE *f=fopen("audio.dat","wb");
			if(!f)return;

			// Synthesize data
			seqr_synthesize(seqr);

			fwrite(seqr->b,sizeof(int16_t),seqr->number_of_samples,f);
			sprintf(seqr->info,"Wrote raw audio to \"audio.dat\"");
			fclose(f);
		}

		// Export WAV file
		if(key=='X')
		{
			// Wave file header tailored for this purpose
			uint8_t hdr[]={0x52, 0x49, 0x46, 0x46, 0xac, 0x58, 0x01, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
				0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x44, 0xac, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00,
				0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x88, 0x58, 0x01, 0x00, 0x0a};

			// Generate, output samples
			FILE *f=fopen("seqexport.wav","wb");
			if(!f)return;

			// Synthesize data
			seqr_synthesize(seqr);

			fwrite(hdr,1,44,f);		// Write WAV header
			fwrite(seqr->b,sizeof(int16_t),seqr->number_of_samples,f);
			sprintf(seqr->info,"Exported \"seqexport.wav\"");
			fclose(f);
		}

		//piano keyboard layout for comp kb
		if(key>=97 && key<=122 &&
			(key!='h'&&key!='j'&&key!='k'&&key!='l'))
		{
			uint32_t freq=0;
			switch(key)
			{
			case 'z':
				freq=131;//c3
				return;
			case 'x':
				freq=147;//d3
				return;
			case 'c':
				freq=165;//e3
				return;
			case 'v':
				freq=175;//f3
				return;
			case 'b':
				freq=196;//g3
				return;
			case 'n':
				freq=220;//a3
				return;
			case 'm':
				freq=247;//b3
				return;
			// case ',':
			case 'q':
				freq=262;//c4
				return;
			case 'w':
				freq=294;//d4
				return;
			case 'e':
				freq=330;//e4
				return;
			case 'r':
				freq=349;//f4
				return;
			case 't':
				freq=392;//g4
				return;
			case 'y':
				freq=440;//a4
				return;
			case 'u':
				freq=494;//b4
				return;
			case 'i':
				freq=523;//c5
				return;
			case 'o':
				freq=587;//d5
				return;
			case 'p':
				freq=659;//e5
				return;
			default:
				return;
			}
			seqr->seq[ui->channel][ui->note].msg=freq;
		}

		// Mark key as read
		key=-1;
		// Clear screen in preparation for redraw
	}
}
