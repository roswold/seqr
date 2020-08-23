#include<stdio.h>
#include"seqr.h"
#include"ui.h"

// Entry
int main(int argc,char**argv)
{
	seqr_data*seqr=seqr_create();

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
	ui_data*ui=ui_create();

	if(!seqr->b)
	{
		mvprintw(1,1,"failed to load buffer\n");
		exit(1);
	}

	// Portaudio setup
	Pa_Initialize();
	Pa_OpenDefaultStream(&seqr->pa,0,1,paInt16,44100,seqr->samples,
		NULL,NULL);

	// Tracker screen
	while(true)
	{

		// Draw tracker note data/UI
		for(int i=seqr->patternoffset;i<seqr->patternoffset+16;i++)
		{
			int yy=0;
			int yinc=8;
			int hilite=seqr->y==i;

			if(hilite)attron(COLOR_PAIR(C_HILITE));

			if(!hilite)attron(COLOR_PAIR(C_GREEN));
			mvprintw(i,yy,"%u",i);
			if(!hilite)attroff(COLOR_PAIR(C_GREEN));

			if(!hilite)attron(COLOR_PAIR(C_WHITE));
			mvprintw(i,yy+=yinc,"%u",seqr->seq[seqr->channel][i].msg);
			if(!hilite)attroff(COLOR_PAIR(C_WHITE));

			if(!hilite)attron(COLOR_PAIR(C_BLUE));
			mvprintw(i,yy+=yinc,"%u",seqr->seq[seqr->channel][i].p1);
			if(!hilite)attroff(COLOR_PAIR(C_BLUE));

			if(!hilite)attron(COLOR_PAIR(C_MAGENTA));
			mvprintw(i,yy+=yinc,"%u\n",seqr->seq[seqr->channel][i].p2);
			if(!hilite)attroff(COLOR_PAIR(C_MAGENTA));

			if(hilite)attroff(COLOR_PAIR(C_HILITE));
		}

		// Draw UI
		{
			int xx=17;
			//setBackgroundColor(BLACK);
			attron(COLOR_PAIR(C_CYAN));
			char*chan_name[]={" (sine)"," (square)"," (triangle)"," (saw)"};
			mvprintw(xx++,0,"Channel:%u%s\n",seqr->channel,chan_name[seqr->channel]);
			attroff(COLOR_PAIR(C_CYAN));

			attron(COLOR_PAIR(C_WHITE));
			mvprintw(xx++,0,"_Q_ Quit\t_W_ Write\t_E_ Edit");
			mvprintw(xx++,0,"_X_ Export\t_R_ Export Raw\tSPACE Play");
			attroff(COLOR_PAIR(C_WHITE));

			// Draw info string if not empty
			if(strcmp(seqr->info,""))
			{
				attron(COLOR_PAIR(C_MAGENTA));
				mvprintw(xx++,0,"\n[%s]",seqr->info);
				attroff(COLOR_PAIR(C_MAGENTA));
			}
		}

		// Check for keyboard input
		{
			int key=getch();
			if(key>0)
			{
				if(key=='Q')break;
				if(key==KEY_UP||key=='k')seqr->y=(seqr->y-seqr->patternoffset-1)%16+seqr->patternoffset;
				if(key==KEY_DOWN||key=='j')seqr->y=(seqr->y+1-seqr->patternoffset)%16+seqr->patternoffset;
				if(key==KEY_LEFT||key=='h')
				{
					if(seqr->channel==0)seqr->channel=3;
					else --seqr->channel;
				}
				if(key==KEY_RIGHT||key=='l')seqr->channel=(seqr->channel+1)%4;
				if(key=='Q')exit(7);

				// Write file
				if(key=='W')
				{
					FILE *f=fopen("seq.dat","wb");
					if(!f)exit(7);
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
					{
						pthread_t th;
						audio_thread_cb_struct s={.pa=seqr->pa,.b=seqr->b,.fc=(seqr->samples)};
						pthread_create(&th,NULL,audio_thread_cb,&s);
					}

					sprintf(seqr->info,"Playing stream");
				}

				// Export raw data
				if(key=='R')
				{
					FILE *f=fopen("audio.dat","wb");
					if(!f)exit(8);

					// Synthesize data
					seqr_synthesize(seqr);

					fwrite(seqr->b,sizeof(int16_t),seqr->samples,f);
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
					if(!f)exit(8);

					// Synthesize data
					seqr_synthesize(seqr);

					fwrite(hdr,1,44,f);		// Write WAV header
					fwrite(seqr->b,sizeof(int16_t),seqr->samples,f);
					sprintf(seqr->info,"Exported \"seqexport.wav\"");
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
					seqr->seq[seqr->channel][seqr->y].msg=fre;
				}

				// Mark key as read
				key=-1;
				// Clear screen in preparation for redraw
				clear();
			}
		}

		// Update ncurses screen
		refresh();
	}

	if(seqr->b)
		free(seqr->b);
	// These called by atexit:
	if(seqr)
		free(seqr);
	if(ui)
		free(ui);
	//Pa_Terminate();
	//endwin();
}

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

seqr_data*seqr_create(void)
{
	seqr_data*seqr;

	// seqr synth stuff
	seqr=malloc(sizeof(seqr_data));
	if(!seqr)printf("failed to allocate seqr_data\n"),exit(3);

	srand(time(NULL));
	seqr->samplerate=44100;
	seqr->samples=seqr->samplerate;
	seqr->fr=261;
	seqr->a=11000;
	seqr->bpm=120;
	memset(seqr->seq,0,sizeof(Msg)*2*16);

	// Allocate buffer for samples
	seqr->b=malloc(sizeof(int16_t)*seqr->samples);
	return seqr;
}

void seqr_synthesize(seqr_data*seqr)
{
	for(int i=0,j=0;i<16;i++)
		for(int key=0;key<2000;key++)
			seqr->b[j]=	(sn(seqr->seq[0][i].msg,j,seqr->samplerate,seqr->seq[0][i].msg?seqr->a:0)+
						sq(seqr->seq[1][i].msg,j,seqr->samplerate,seqr->seq[1][i].msg?seqr->a:0)+
						tr(seqr->seq[2][i].msg,j,seqr->samplerate,seqr->seq[2][i].msg?seqr->a:0)+
						sw(seqr->seq[3][i].msg,j,seqr->samplerate,seqr->seq[3][i].msg?seqr->a:0)
						)/4.0,++j;
}
