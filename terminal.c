//===========================================================================
// FILE: terminal.c
//===========================================================================
// Implementation file for terminal functions
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// Allow changing colour, etc (using ANSI escape codes).
//---------------------------------------------------------------------------

#include <stdio.h>
#include "terminal.h"


void terminal_settextcolour (char* code, int bright)
{
	if (bright)	printf("\x1b[%s;1m", code);
	else        printf("\x1b[%sm",   code);
}
void terminal_settextblack   (int bright) {terminal_settextcolour("30",bright);}
void terminal_settextred     (int bright) {terminal_settextcolour("31",bright);}
void terminal_settextgreen   (int bright) {terminal_settextcolour("32",bright);}
void terminal_settextyellow  (int bright) {terminal_settextcolour("33",bright);}
void terminal_settextblue    (int bright) {terminal_settextcolour("34",bright);}
void terminal_settextmagenta (int bright) {terminal_settextcolour("35",bright);}
void terminal_settextcyan    (int bright) {terminal_settextcolour("36",bright);}
void terminal_settextwhite   (int bright) {terminal_settextcolour("37",bright);}
void terminal_reset()         {printf("\x1b[0m");}
