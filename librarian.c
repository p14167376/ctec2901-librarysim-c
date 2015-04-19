//===========================================================================
// FILE: librarian.c
//===========================================================================
// Implementation file for librarian code
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
#include "smalloc.h"
#include "trace.h"
#include "shutdown.h"
#include "set_ints.h"
#include "msg_queue.h"
#include "librarian.h"
#include "library.h"


#define LIBRARIAN_NUMBOOKSTOADD   60
#define LIBRARIAN_DELAY         2000 // milliseconds
#define LIBRARIAN_MAXBOOKSRQST     5
#define LIBRARIAN_MAXLOANSRQST     5

typedef struct
{
	msg_client_t* client;
	int           numBorrowers;
} librarian_t;

void librarian_ADD (librarian_t* lbrn)
{
	assert(lbrn != NULL);
	list* templist = new_list(set_ints_compare);

	int n;
	for (n=0; n<LIBRARIAN_NUMBOOKSTOADD; n++)
	{
		long id = rand()%LIBRARY_MAXBOOKIDS;
		list_ins_after(templist, (any)id);
	}
	msg_client_send (lbrn->client, "ADD", (any)templist);

	list_release(templist);
}

void librarian_BOOKS (librarian_t* lbrn)
{
	assert(lbrn != NULL);
	set* tempset = set_ints_create();

	int n;
	int max = (rand() % (LIBRARIAN_MAXBOOKSRQST - 1)) + 1;
	for (n=0;n<max; n++)
	{
		long id = rand()%LIBRARY_MAXBOOKIDS;
		set_insertInto(tempset, (any)id);
	}
	msg_client_send (lbrn->client, "BOOKS", tempset);

	set_ints_release(tempset);
}

void librarian_LOANS (librarian_t* lbrn)
{
	assert(lbrn != NULL);
	set* tempset = set_ints_create();

	int n;
	int max = (rand() % (LIBRARIAN_MAXLOANSRQST - 1)) + 1;
	for (n=0;n<max; n++)
	{
		long id = rand() % lbrn->numBorrowers;
		set_insertInto(tempset, (any)id);
	}
	msg_client_send (lbrn->client, "LOANS", tempset);

	set_ints_release(tempset);
}

void* librarian_run (void* arg)
{
	assert(arg != NULL);

	librarian_t lbrn;
	lbrn.client = msg_client_create ((msg_queue_t*)arg);

	msg_client_send (lbrn.client, "GETNB", (any)(&lbrn.numBorrowers));

	librarian_ADD (&lbrn);

	int action;
	while (!shutdown)
	{
		millisleep_allowing_shutdown (LIBRARIAN_DELAY);

		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0: librarian_BOOKS(&lbrn); break;
				case 1: librarian_LOANS(&lbrn); break;
					break;
			}
		}
	}

	msg_client_release (lbrn.client);
}
