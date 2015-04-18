//===========================================================================
// FILE: librarian.c
//===========================================================================
// Implementation file for librarian code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------


// Standard C Headers
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Standard CTEC2901 Headers
#include "clist.h"
#include "queue_any.h"

// Project Headers
#define TRACE_ON
#include "smalloc.h"
#include "trace.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "librarian.h"



void* librarian_run (void* arg)
{
	msg_client_t* client = msg_client_create ((msg_queue_t*)arg);

	while (!shutdown)
	{
		sleep(1);
	}

	msg_client_release (client);
}