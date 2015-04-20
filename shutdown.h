//===========================================================================
// FILE: shutdown.h
//===========================================================================
// Header file for shutdown functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// 1. A global variable to indicate shutdown.
// Thread safety not an issue (only read or set to 1 after initialisation)
// 2. Sleep functions which allow for shutdown
// Sleep for short period, check shutdown, repeat until full period elapsed.
//---------------------------------------------------------------------------
#ifndef SHUTDOWN_H
#define SHUTDOWN_H

extern int shutdown;
void sleep_allowing_shutdown (int seconds);
void millisleep_allowing_shutdown (int milliseconds);

#endif//SHUTDOWN_H