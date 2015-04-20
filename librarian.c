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
#include "sim.h"


typedef struct
{
	msg_client_t* client;
	int           numBorrowers;
} librarian_t;

void librarian_ADD (librarian_t* lbrn)
{
	assert(lbrn != NULL);

	// create a list of random books to add and send to library
	list* templist = new_list(set_ints_compare);
	int n;
	for (n=0; n<config.lbryNumBooks; n++)
	{
		long id = rand() % config.lbryBookRange;
		list_ins_after(templist, (any)id);
	}
	msg_client_send (lbrn->client, "ADD", (any)templist);

	list_release(templist);
}

void librarian_BOOKS (librarian_t* lbrn)
{
	assert(lbrn != NULL);

	// create a list of random books and ask for their status
	set* tempset = set_ints_create();
	int n;
	int max = (rand() % (config.lbrnRqstSize - 1)) + 1;
	for (n=0;n<max; n++)
	{
		long id = rand() % config.lbryBookRange;
		set_insertInto(tempset, (any)id);
	}
	msg_client_send (lbrn->client, "BOOKS", tempset);

	set_ints_release(tempset);
}

void librarian_LOANS (librarian_t* lbrn)
{
	assert(lbrn != NULL);

	// create a list of random borrowers and ask for their status
	set* tempset = set_ints_create();
	int n;
	int max = (rand() % (config.lbrnRqstSize - 1)) + 1;
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
	library_t* lib = (library_t*)arg;

	// set up the librarian details and get the number of borrowers
	librarian_t lbrn;
	lbrn.client = msg_client_create (library_getqueue(lib));
	msg_client_send (lbrn.client, "GETNB", (any)(&lbrn.numBorrowers));

	// seed library with some books
	librarian_ADD (&lbrn);

	// until shutdown we loop, sending random messages
	int action;
	while (!shutdown)
	{
		millisleep_allowing_shutdown (config.lbrnDelay);

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
