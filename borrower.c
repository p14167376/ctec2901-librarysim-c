//===========================================================================
// FILE: borrower.c
//===========================================================================
// Implementation file for borrower code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// Notes:
//  Rather than passing an id into each borrower, the borrowers make an
//  initial message call to the library to register and get an id value.
//  This is a reflection of how individuals register with a real library.
//---------------------------------------------------------------------------


// Standard C Headers
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// ds library headers
#include "list.h"
#include "set.h"

// Project Headers
#define TRACE_ON
#include "smalloc.h"
#include "trace.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "borrower.h"
#include "library.h"


#define BORROWER_DELAY        600 // milliseconds
#define BORROWER_MAXBOOKSRQST 5

typedef struct
{
	int           id;
	msg_client_t* client;
	set*          myBooks;
} borrower_t;

extern void int_printer(any x);
extern int  int_compare(any x, any y);

void borrower_RQST(borrower_t* brwr)
{
	assert(brwr != NULL);
	set* tempset = new_set (int_printer, int_compare);

	int n;
	int max = (rand() % (BORROWER_MAXBOOKSRQST - 1)) + 1;
	for (n=0;n<max; n++)
	{
		long id = rand()%LIBRARY_MAXBOOKIDS;
		if (!set_isin(brwr->myBooks, (any)id))
		{
			set_insertInto(tempset, (any)id);
		}
	}
	printf ("BORROWER %d: Requested Books ", brwr->id);
	set_print(tempset); printf("\n");

	msg_client_send (brwr->client, "RQST", (any)tempset);

	printf ("BORROWER %d: Received Books ", brwr->id);
	set_print(tempset); printf("\n");

	set_unionWith(brwr->myBooks, tempset);

	while(!set_isempty(tempset)) set_choose_item(tempset);
	set_release(tempset);
}

void borrower_RTRN(borrower_t* brwr)
{
	printf ("BORROWER %d: Send RTRN\n", brwr->id);
	msg_client_send (brwr->client, "RTRN", (any)brwr->myBooks);
}

void* borrower_run (void* arg)
{
	assert(arg != NULL);

	borrower_t brwr;
	brwr.id      = -1;
	brwr.myBooks = new_set(int_printer, int_compare);
	brwr.client  = msg_client_create ((msg_queue_t*)arg);

	msg_client_send (brwr.client, "RGST", (any)(&brwr.id));

	int action;
	while (!shutdown)
	{
		millisleep_allowing_shutdown (BORROWER_DELAY);

		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0: borrower_RQST (&brwr); break;
				case 1: borrower_RTRN (&brwr); break;
					break;
			}
		}
	}

	msg_client_release (brwr.client);
	while(!set_isempty(brwr.myBooks)) set_choose_item(brwr.myBooks);
	set_release (brwr.myBooks);
}
