#include<stdio.h>
#include"seqr.h"
#include"ui.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#	include<pdcurses.h>
#else
#	include<ncurses.h>
#endif

// Entry
int main(int argc,char**argv)
{
	ui_data*ui;
	seqr_data*seqr;

	// Parse command line options
	for(int i=0;i<argc;++i)
		if(strcmp(argv[i],"--help")==0 || strcmp(argv[i],"-h")==0)
		{
			puts("seqr version 0.0\nusage: seqr [--help|-h]");
			exit(0);
		}

	// Initialize state
	ui=ui_create();
	seqr=seqr_create();
	signal(SIGINT,sighandler);

	// Main loop
	while(ui->running)
	{

		// Draw UI
		clear();
		seqr_drawnotes(seqr,ui);
		seqr_drawui(seqr,ui);

		// Check for keyboard input
		seqr_kb(seqr,ui);

		// Update ncurses screen
		refresh();
	}

	// Exit, clear resources
	seqr_close(seqr);
	ui_close(ui);
}
