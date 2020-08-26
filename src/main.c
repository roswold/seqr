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
		seqr_kb(seqr,ui);

		// Update ncurses screen
		refresh();
	}

quit:
	seqr_close(seqr);
	ui_close(ui);
}
