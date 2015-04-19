//===========================================================================
// FILE: shutdown.c
//===========================================================================
// Implementation file for shutdown functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// Provides delay functions that respond to shutdown at intervals
// NOTE: these are not accurate as additional time stopping & starting.
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
	}
	if (!shutdown) nanosleep(&delay, &timeleft);
}
