#include"ui.h"

// Allocate, initialize ui_data 
ui_data*ui_create(void)
{
	ui_data*ui=(ui_data*)malloc(sizeof(ui_data));
	// ncurses setup
	ui->w=initscr();
	keypad(ui->w, TRUE);
	start_color();
	use_default_colors();
	curs_set(0);
	ui->note=0;
	ui->channel=0;
	ui->running=1;

	init_pair(1,COLOR_CYAN,C_TRANSPARENT);
	init_pair(2,COLOR_GREEN,C_TRANSPARENT);
	init_pair(3,COLOR_YELLOW,C_TRANSPARENT);
	init_pair(4,COLOR_WHITE,C_TRANSPARENT);
	init_pair(5,COLOR_MAGENTA,C_TRANSPARENT);
	init_pair(6,COLOR_BLUE,C_TRANSPARENT);
	init_pair(7,COLOR_BLACK,COLOR_WHITE);
	return ui;
}

void ui_close(ui_data*ui)
{
	endwin();
	if(ui)free(ui);
}
