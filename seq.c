#include"rlutil.h"
#include<stdlib.h>
#include<stdio.h>
#include<inttypes.h>
#include<math.h>
#include<time.h>

#define SEMITC pow(2,1/12.0)
#define raisesemi(f,n) (f*pow(SEMITC,(n)))
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


#define MSG_OFF	0x00
#define MSG_ON	0x01
typedef struct Msg
{
	uint32_t msg,p1,p2;
} Msg;

int main(int argc,char **argv)
{
	
	// if(argc<2)
	// {
		// printf("usage: %s <output filename>\n",*argv);
		// exit(2);
	// }
	
	atexit(cls);
	atexit(resetColor);
	atexit(showcursor);
	
	hidecursor();
	saveDefaultColor();
	setConsoleTitle(*argv);
	
	
	//sy stuff
	srand(time(NULL));
	uint32_t samplerate=44100;
	uint32_t samples=samplerate;
	double fr=261;
	double a=11000;
	int32_t bpm=120;
	
	//seq stuff
	Msg seq[2][16];
	uint32_t y=0;
	char info[64]={0};
	uint8_t channel=0;
	
	
	int16_t *b=malloc(sizeof(int16_t)*samples);
	if(!b)
	{
		gotoxy(1,1);
		anykey("failed to load buffer\n");
		exit(1);
	}
	
	
	cls();
	gotoxy(0,0);
	setColor(CYAN);
	printf("%21s\n",*argv);
	setColor(MAGENTA);
	printf("%21s\n%21s","[Press any key]","----------------");
	anykey("\n");
	
	
	//update loop
	while (1)
	{
		if (kbhit()) //minimal update
		{
			char k = getkey();
			
			if(k==KEY_ESCAPE)break;
			if(k==KEY_UP)y=--y%16;
			if(k==KEY_DOWN)y=++y%16;
			if(k==KEY_LEFT)channel=(channel-1)%2;
			if(k==KEY_RIGHT)channel=(channel+1)%2;
			if(k=='Q')exit(7);
			if(k=='S')
			{
				FILE *f=fopen("seq.dat","wb");
				if(!f)exit(7);
				fwrite(seq,1,sizeof(seq),f);
				gotoxy(0,0);
				sprintf(info,"saved \"seq.dat\"");
				fclose(f);
			}
			if(k=='O')
			{
				FILE *f=fopen("seq.dat","rb");
				if(!f)exit(7);
				fread(seq,1,sizeof(seq),f);
				gotoxy(0,0);
				sprintf(info,"opened \"seq.dat\"");
				fclose(f);
			}
			if(k=='E')
			{
				FILE *f=fopen("audio.dat","wb");
				if(!f)exit(8);
				
				int samp=0;
				// for(int i=0;i<samples;i++)
					// b[i]=sw(fr,i,samplerate,a);
				for(int i=0,j=0;i<16;i++)
					for(int k=0;k<1000;k++)
						b[j]=(	sw(seq[0][i].msg,j  ,samplerate,seq[0][i].msg?a:0)+
								tr(seq[1][i].msg,j++,samplerate,seq[1][i].msg?a:0)
								)/2.0;
							
				
				fwrite(b,sizeof(int16_t),samples,f);
				gotoxy(0,0);
				sprintf(info,"exported \"audio.dat\"");
				fclose(f);
			}
			
			//piano keyboard layout for comp kb
			if(k>=97 && k<=122)
			{
				uint32_t fre=0;
				switch(k)
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
				seq[channel][y].msg=fre;
			}
			
			cls();
			for(int i=0;i<16;i++)
			{
				gotoxy(0,i+1);
				if(y==i)setBackgroundColor(BLUE);
				else setBackgroundColor(BLACK);
				
				setColor(DARKGREY);
				printf("%4u",i);
				setColor(WHITE);
				printf("%12u",seq[channel][i].msg);
				setColor(CYAN);
				printf("%12u",seq[channel][i].p1);
				setColor(MAGENTA);
				printf("%12u\n",seq[channel][i].p2);
				
			}
			
			setBackgroundColor(BLACK);
			setColor(GREY);
			gotoxy(0,19);
			setColor(BLUE);
			printf("Channel:%u",channel);
			setColor(GREY);
			printf("\n%12s%12s%12s","_Q_:Quit","_S_:Save","_O_:Open");
			printf("\n%12s","_E_:Export");
			setColor(DARKGREY);
			printf("\n[%s]",info);
		}
	}
	return 0;
}
