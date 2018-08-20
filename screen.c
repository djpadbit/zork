#include "screen.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <display.h>
#include <clock.h>
#include <stdarg.h>
#include <keyboard.h>

int sc_init()
{
	//init vars
	//Stack overflow gud:
	//	https://stackoverflow.com/questions/1970698/using-malloc-for-allocation-of-multi-dimensional-arrays-with-different-row-lengt
	int i;
	if (( sc_screenContent = (char**)malloc( SCR_NBRLINES*sizeof( char* ))) == NULL ) return 0;
	for (i=0; i < SCR_NBRLINES; i++ ) { if (( sc_screenContent[i] = (char*)malloc( SCR_CHARSPERLINE+1 )) == NULL ) return 0; }

	for(i=0;i<SCR_NBRLINES;i++)
	{
		sc_screenContent[i][0]=0;
	}
	sc_actualLine = 0;
	sc_scrindex = 0;
	sc_chrindex = 0;
	//rockback = 1;
	//init screen
	dclear();
	text_configure(&res_font_modern, color_black);
	keyboard_setRepeatRate(625,50);
	return 1;
}

void sc_cleanup()
{
	for (int i=0;i<SCR_NBRLINES;i++) free(sc_screenContent[i]);
	free(sc_screenContent);
}

void sc_print(const char* fmt, ...)
{
	char k[128];
	va_list args;
	va_start(args, fmt);
	vsprintf(k,fmt,args);
	va_end(args);
	for (int i=0;i<strlen(k);i++) sc_write(k[i]);
	//sc_writeLine(k);
}

void sc_writeLine(char* str)
{
	if(sc_screenContent[sc_actualLine][0]!=0)
		sc_newline();
	sc_setString(str,sc_actualLine);
	sc_newline();
	//sc_rewritescreen();
}
//unsigned char tempvar;
void sc_setString(char* str, unsigned char line)
{
	int j,disp;
	//tempvar = line;
	for(j=0;j<SCR_CHARSPERLINE;j++)
	{
		if(str[j] == 0)
		{
			break;
		}
		char cur = str[j];
		sc_screenContent[line][j]=cur;
		char str[] = {cur,0};
		  //int(line/float(SCR_DSPLINES)) <= sc_scrindex
		if (line < sc_scrindex+SCR_DSPLINES && line > sc_scrindex) {dtext(j*6,6*(line%SCR_DSPLINES),(unsigned char*)str); disp = 1;}
	}
	sc_screenContent[line][j] = 0;
	sc_chrindex = j;
	if (disp) {
		dupdate();
		//ML_display_vram_lines(6*(line%SCR_DSPLINES),6*((line%SCR_DSPLINES)+1));
		//if (sc_scrindex != int(float(sc_actualLine)/float(SCR_DSPLINES))) sc_setScrindex(int(float(sc_actualLine)/float(SCR_DSPLINES)));
		//if (sc_scrindex > sc_actualLine || sc_scrindex+(SCR_DSPLINES-1) < sc_actualLine ) sc_setScrindex(sc_actualLine-(SCR_DSPLINES-1));
		if (sc_scrindex < sc_actualLine && sc_actualLine > (SCR_DSPLINES-1) || sc_scrindex+(SCR_DSPLINES-1) > sc_actualLine && sc_actualLine > (SCR_DSPLINES-1) ) sc_setScrindex(sc_actualLine-(SCR_DSPLINES-1));
	}
}

void sc_setactualLine(unsigned char line)
{
	if((int)line >= 0 && (int)line < SCR_NBRLINES)
	{
		sc_actualLine = line;
	}
}

void sc_setScrindex(int newscr)
{
	sc_scrindex = newscr;
	sc_rewritescreen();
}

void sc_backspace()
{
	if (sc_chrindex == 0 && sc_actualLine != 0) {
		sc_actualLine--;
		sc_chrindex = SCR_CHARSPERLINE-1;
	} else sc_chrindex--;
}

void sc_bell()
{
	drect(126,0,128,2,color_black);
	dupdate();
	sleep_ms(50);
	drect(126,0,128,2,color_white);
	dupdate();
}

static int lower = 1;
static int shift = 0;
static int ls = 0;
static int alpha = 0;
static int la = 0;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

char* sc_gets(char* str,int len)
{
	int key,ptr;
	ptr = 0;
	key = 0;
	int run = 1;
	while (run) {
		key = getkey_opt(getkey_repeat_all_keys,0);
		switch (key) {
			/*case KEY_LEFT:
				if (ptr > 0) ptr--;
				break;
			case KEY_RIGHT: 
				if (str[ptr] != 0) ptr++;
				break;*/
			case KEY_UP:
				if (sc_scrindex>0) sc_setScrindex(sc_scrindex-1);
				break;
			case KEY_DOWN:
				if (sc_scrindex<MAX(sc_actualLine-(SCR_DSPLINES-1),0)) sc_setScrindex(sc_scrindex+1);
				break;
			case KEY_EXE:
				if (ptr != 0) run = 0;
				break;
			case KEY_DEL: 
				if (ptr > 0) {str[ptr--] = 0;str[ptr] = 0;sc_write(0x8);sc_write(' ');sc_write(0x8);}
				break;
			case KEY_F1:
				lower = !lower;
				break;
			case KEY_SHIFT:
				shift = !shift;
				break;
			case KEY_ALPHA:
				if (shift) alpha = 2;
				else alpha = !alpha;
				break;
			default:
				if (alpha) key |= MOD_ALPHA;
				if (shift) key |= MOD_SHIFT;
				if (key_char(key) != 0 && ptr < len) { 
					str[ptr++] = lower ? tolower(key_char(key)) : key_char(key);
					sc_write(lower ? tolower(key_char(key)) : key_char(key));
				}
				break;
		}
		if (ls) shift = 0;
		if (la == 1) alpha = 0;
		ls = shift;
		la = alpha;
	}
	str[ptr] = 0;
	sc_write('\n');
	return str;
}

void sc_write(char data)
{
	//verify if the char is a carrage return or a line feed
	if (data == 0xD) sc_chrindex = 0;
	else if (data == 0xA) sc_newline();
	else if (data == 0x7) sc_bell();
	else if (data == 0x8) sc_backspace();
	else
	{
		if(sc_chrindex >= SCR_CHARSPERLINE)
		{
			sc_newline();
			sc_chrindex = 0;
		}
		//Add the new char
		sc_screenContent[sc_actualLine][sc_chrindex] = data;
		//sc_screenContent[sc_actualLine][sc_chrindex+1] = 0;
		//Writing
		if (sc_scrindex < sc_actualLine && sc_actualLine > (SCR_DSPLINES-1) /*&& rockback*/ || sc_scrindex+(SCR_DSPLINES-1) > sc_actualLine && sc_actualLine > (SCR_DSPLINES-1) /*&& rockback*/ ) sc_setScrindex(sc_actualLine-(SCR_DSPLINES-1));
		//if (sc_scrindex > sc_actualLine && sc_actualLine <= 10) sc_setScrindex(0);
		//if (sc_scrindex > sc_actualLine) sc_setScrindex(sc_actualLine);
		//else if (sc_scrindex+(SCR_DSPLINES-1) < sc_actualLine) sc_setScrindex(sc_actualLine-(SCR_DSPLINES-1));
		else/* if (sc_actualLine > sc_scrindex && sc_actualLine < sc_scrindex+(SCR_DSPLINES-1))*/ {
			char str[] = {data,0};
			/*if (int(sc_actualLine/float(10)) <= sc_scrindex) */dtext(sc_chrindex*6,6*(sc_actualLine%SCR_DSPLINES),(unsigned char*)str);
			//ML_display_vram_lines(6*(sc_actualLine%SCR_DSPLINES),6*((sc_actualLine%SCR_DSPLINES)+1));
			dupdate();
		}
		sc_chrindex++;
		//int k = sc_actualLine % SCR_DSPLINES;
		//char str[] = {data,0};
		//PrintMini(longueur*6,6*k,(unsigned char*)str,MINI_OVER);
		//ML_display_vram();
	}
}

void sc_clear()
{
	dclear();
	for(int i=0;i<SCR_NBRLINES-1;i++)
	{
		sc_screenContent[i][0] = 0;
	}
	sc_actualLine=0;
	sc_setScrindex(0);
}

void sc_newline()
{
	if(sc_actualLine>=SCR_NBRLINES-1)
	{
		//shifts the lines to free the last
		for(int i=0;i<SCR_NBRLINES-1;i++)
		{
			for(int j=0;j<SCR_CHARSPERLINE;j++)
			{
				if(sc_screenContent[i+1][j] == 0)
				{
					sc_screenContent[i][j] = 0;
					break;
				}
				sc_screenContent[i][j]=sc_screenContent[i+1][j];
			}
		}
		memset(sc_screenContent[sc_actualLine],0,SCR_CHARSPERLINE);
		//sc_screenContent[sc_actualLine][0] = 0;
		sc_chrindex = 0;
		sc_rewritescreen();
	}
	else
	{
		sc_actualLine++;
		sc_screenContent[sc_actualLine][0] = 0;
		sc_chrindex = 0;
		//sc_rewritescreen();
	}
}


void sc_rewritescreen()
{
	dclear();
	for(int i=0;i<SCR_DSPLINES;i++)
	{
		for(int j=0;j<SCR_CHARSPERLINE;j++)
		{
			int k = i+sc_scrindex;//(SCR_DSPLINES*sc_scrindex);//(SCR_DSPLINES*int(float(sc_actualLine)/float(SCR_DSPLINES)));
			//char cur = sc_screenContent[][j]
			if(sc_screenContent[k][j] == 0)
				break;
			char str[] = {sc_screenContent[k][j],0};
			dtext(j*6,6*i,(unsigned char*)str);
		}
		// PrintMini(0,6*i,(unsigned char*)sc_screenContent[i],MINI_OVER);
	}
	dupdate();
}