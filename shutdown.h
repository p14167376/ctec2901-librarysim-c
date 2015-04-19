//===========================================================================
// FILE: shutdown.h
//===========================================================================
// Header file for shutdown functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef SHUTDOWN_H
#define SHUTDOWN_H

extern int shutdown;
void sleep_allowing_shutdown (int seconds);
void millisleep_allowing_shutdown (int milliseconds);

#endif//SHUTDOWN_H