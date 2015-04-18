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



struct librarian_impl
{
	msg_client_t* client;
};

librarian_t* librarian_create  (msg_queue_t* msgq)
{
	SAFE_MALLOC(librarian_t, lbrn);
	lbrn->client = msg_client_create (msgq);
}

void librarian_release (librarian_t* lbrn)
{
	msg_client_release (lbrn->client);
}
