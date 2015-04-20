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
#include "set_ints.h"
#include "msg_queue.h"
#include "borrower.h"
#include "library.h"


#define BORROWER_DELAY        2000 // milliseconds
#define BORROWER_MAXBOOKSRQST    5

typedef struct
{
	int           id;
	msg_client_t* client;
	set*          myBooks;
} borrower_t;

void borrower_RQST(borrower_t* brwr)
{
	assert(brwr != NULL);
	set* tempset = set_ints_create();

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

	library_RQST_t librq;
	librq.brwr  = brwr->id;
	librq.books = tempset;
	msg_client_send (brwr->client, "RQST", (any)&librq);

	set_unionWith(brwr->myBooks, tempset);

	set_ints_release(tempset);
}

void borrower_RTRN(borrower_t* brwr)
{
	assert(brwr != NULL);
	if(set_count(brwr->myBooks) > 0)
	{
		library_RQST_t librq;
		librq.brwr  = brwr->id;
		librq.books = brwr->myBooks;
		msg_client_send (brwr->client, "RTRN", (any)&librq);
		set_ints_removeall(brwr->myBooks);
	}
}

void* borrower_run (void* arg)
{
	assert(arg != NULL);
	library_t* lib = (library_t*)arg;

	borrower_t brwr;
	brwr.id      = -1;
	brwr.myBooks = set_ints_create();
	brwr.client  = msg_client_create (library_getqueue(lib));

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
	set_ints_release (brwr.myBooks);
}
