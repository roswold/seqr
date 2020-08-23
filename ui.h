#include<ncurses.h>
#include<stdlib.h>
#include<stdio.h>

// Struct to define UI information using ncurses
typedef struct ui_data
{
	WINDOW*w;
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
