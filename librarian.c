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

// Project Headers
#include "smalloc.h"
#include "trace.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "librarian.h"
#include "library.h"


#define LIBRARIAN_DELAY  5

extern int int_compare(any x, any y);

typedef struct
{
	msg_client_t* client;
} librarian_t;

//typedef enum {ACTION_BOOKS_REQUEST, ACTION_LOANS_REQUEST} action_t;

void generate_random_books (librarian_t* lbrn)
{
	assert(lbrn != NULL);

	list* newbooks = new_list(int_compare);

	int n;
	srand(time(NULL));
	for (n=0; n<60; n++)
	{
		long id = rand()%LIBRARY_MAXBOOKIDS;
		list_ins_after(newbooks, (any)id);
	}

	msg_client_sendpayload (lbrn->client, "ADD", (any)newbooks);
	list_release(newbooks);
}

void librarian_simplemsg (librarian_t* lbrn, char* msgName)
{
	assert(lbrn != NULL);
	assert(msgName != NULL);

	//printf ("LIBRARIAN: Send %s request\n", msgName);
	msg_client_send (lbrn->client, msgName);
}

void* librarian_run (void* arg)
{
	assert(arg != NULL);

	librarian_t lbrn;
	lbrn.client = msg_client_create ((msg_queue_t*)arg);

	generate_random_books (&lbrn);

	int action;
	while (!shutdown)
	{
		delay_allowing_shutdown (LIBRARIAN_DELAY);

		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0: librarian_simplemsg(&lbrn, "BOOKS"); break;
				case 1: librarian_simplemsg(&lbrn, "LOANS"); break;
			}
		}
	}

	msg_client_release (lbrn.client);
}
