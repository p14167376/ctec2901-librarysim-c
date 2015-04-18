//===========================================================================
// FILE: borrower.c
//===========================================================================
// Implementation file for borrower code
// Author: Barnaby Stewart (P14167376)
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


#define BORROWER_DELAY        6
#define BORROWER_MAXBOOKSRQST 5

typedef struct
{
	msg_client_t* client;
	set*          myBooks;
} borrower_t;

void int_printer(any x) {printf("%d",(long)x);}
int  int_compare(any x, any y)
{
	if (x < y) return -1;
	if (x > y) return  1;
	return  0;
}

void borrower_RQST(borrower_t* brwr)
{
	assert(brwr != NULL);
	set* tempset = new_set (int_printer, int_compare);

	int n;
	int max = rand() % BORROWER_MAXBOOKSRQST;
	for (n=0;n<max; n++)
	{
		long id = rand()%LIBRARY_MAXBOOKIDS;
		if (!set_isin(brwr->myBooks, (any)id))
		{
			set_insertInto(tempset, (any)id);
		}
	}
	printf ("BORROWER: Requested Books ");
	set_print(tempset); printf("\n");

	msg_client_send (brwr->client, "RQST", (any)tempset);

	printf ("BORROWER: Received Books ");
	set_print(tempset); printf("\n");

	// TODO record which books were loaned
	set_unionWith(brwr->myBooks, tempset);

	while(!set_isempty(tempset)) set_choose_item(tempset);
	set_release(tempset);
}

void borrower_RTRN(borrower_t* brwr)
{
	printf ("BORROWER: Send RTRN\n");
	msg_client_send (brwr->client, "RTRN", (any)brwr->myBooks);
}

void* borrower_run (void* arg)
{
	assert(arg != NULL);

	borrower_t brwr;
	brwr.myBooks = new_set(int_printer, int_compare);
	brwr.client = msg_client_create ((msg_queue_t*)arg);

	int action;
	while (!shutdown)
	{
		delay_allowing_shutdown (BORROWER_DELAY);

		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0: borrower_RQST (&brwr); break;
				case 1: borrower_RTRN (&brwr); break;
			}
		}
	}

	msg_client_release (brwr.client);
	set_release (brwr.myBooks);
}
