#ifndef SCREEN_H
#define SCREEN_H
#include <display.h>

#define SCR_DSPLINES 10
#define SCR_NBRLINES 200
#define SCR_CHARSPERLINE 64

extern font_t res_font_modern;

int sc_init();
void sc_cleanup();
void sc_rewritescreen();
void sc_write(char data);
void sc_clear();
void sc_backspace();
void sc_newline();
void sc_bell();
char* sc_gets(char* str,int len);
void sc_setString(char* str, unsigned char line);
void sc_print(const char* fmt, ...);
void sc_writeLine(char* str);
void sc_setActualLine(unsigned char line);
void sc_setScrindex(int newscr);
static int sc_actualLine;
static int sc_scrindex;
static int sc_chrindex;
static char** sc_screenContent;


#endif // SCREEN_H