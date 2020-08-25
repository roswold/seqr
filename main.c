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
	while(ui->running)
	{

		// Draw UI
		clear();	// Clear screen in preparation for redraw
		seqr_drawnotes(seqr,ui);
		seqr_drawui(seqr,ui);

		// Check for keyboard input
		//seqr_kb(seqr,ui);
		if(!ui->running)break;
		int key=getch();
		//TODO: change these if statements into a single switch statement with falls-through
		if(key>0)
		{

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
				seqr_write_file(seqr,"seq.dat");

			// Edit existing file
			if(key=='E')
				seqr_edit_file(seqr,"seq.dat");

			// Space --> Play audio
			if(key==' ')
				seqr_play(seqr);

			// Export raw data
			if(key=='R')
				seqr_write_raw_file(seqr,"audio.dat");

			// Export WAV file
			if(key=='X')
				seqr_export(seqr,"seqexport.wav");

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
		}

		// Update ncurses screen
		refresh();
	}

quit:
	seqr_close(seqr);
	ui_close(ui);
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
	//if(!f)break;
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
