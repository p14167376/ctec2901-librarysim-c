//===========================================================================
// FILE: terminal.h
//===========================================================================
// Header file for terminal functions
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef TERMINAL_H
#define TERMINAL_H


void terminal_settextcolour  (char* code, int bright);
void terminal_settextblack   (int bright);
void terminal_settextred     (int bright);
void terminal_settextgreen   (int bright);
void terminal_settextyellow  (int bright);
void terminal_settextblue    (int bright);
void terminal_settextmagenta (int bright);
void terminal_settextcyan    (int bright);
void terminal_settextwhite   (int bright);
void terminal_reset();


#endif//TERMINAL_H