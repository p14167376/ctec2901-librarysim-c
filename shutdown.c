//===========================================================================
// FILE: shutdown.c
//===========================================================================
// Implementation file for shutdown functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// 1. A global variable to indicate shutdown.
// Thread safety not an issue (only read or set to 1 after initialisation)
// 2. Sleep functions which allow for shutdown
// Sleep for short period, check shutdown, repeat until full period elapsed.
// NOTE: Not strictly accurate as additional time used stopping & starting.
// (but good enough for the purposes of the simulation)
//---------------------------------------------------------------------------

#include <time.h>
#include "shutdown.h"

int shutdown = 0;

void sleep_allowing_shutdown (int delay)
{
	int delayLoop;
	for(delayLoop=0;delayLoop<delay;delayLoop++)
	{
		if (!shutdown) sleep(1);
		else return;
	}
}

void millisleep_allowing_shutdown (int milliseconds)
{
    struct timespec delay, timeleft;
    delay.tv_sec  = 0;
    delay.tv_nsec = (milliseconds % 1000) * 1000000;
    int seconds   = milliseconds / 1000;

	int delayLoop;
	for(delayLoop=0;delayLoop<seconds;delayLoop++)
	{
		if (!shutdown) sleep(1);
		else return;
	}
	if (!shutdown) nanosleep(&delay, &timeleft);
}
