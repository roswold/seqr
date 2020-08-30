#pragma once
#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#	include<pdcurses.h>
#else
#	include<ncurses.h>
#endif

// Struct to define UI information using ncurses
typedef struct ui_data
{
	WINDOW*w;
	int note;
	int channel;
	int pattern;
	int running;
} ui_data;

// ncurses colors enum
enum colors
{
	C_TRANSPARENT=-1,
	C_CYAN=1,
	C_GREEN,
	C_YELLOW,
	C_WHITE,
	C_MAGENTA,
	C_BLUE,
	C_HILITE
};

// Allocate, initialize ui_data 
ui_data*ui_create(void);

// Free ui_data resources
void ui_close(ui_data*ui);
