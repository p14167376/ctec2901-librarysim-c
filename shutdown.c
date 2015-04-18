//===========================================================================
// FILE: shutdown.c
//===========================================================================
// Implementation file for shutdown functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------

#include "shutdown.h"

int shutdown = 0;

// Provides a delay function that responds to shutdown at intervals
void delay_allowing_shutdown (int delay)
{
	int delayLoop;
	for(delayLoop=0;delayLoop<delay;delayLoop++)
	{
		if (!shutdown) sleep(1);
	}
}
