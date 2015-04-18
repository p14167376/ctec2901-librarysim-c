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
#include "msg_queue.h"
#include "librarian.h"
#include "library.h"


#define LIBRARIAN_DELAY        5
#define LIBRARIAN_MAXBOOKSRQST 5
#define LIBRARIAN_MAXLOANSRQST 5

extern void int_printer(any x);
extern int  int_compare(any x, any y);

typedef struct
{
	msg_client_t* client;
} librarian_t;

void librarian_ADD (librarian_t* lbrn)
{
	assert(lbrn != NULL);
	printf ("LIBRARIAN: Send ADD\n");

	list* templist = new_list(int_compare);

	int n;
	for (n=0; n<60; n++)
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
	//printf ("LIBRARIAN: Send BOOKS\n");
	char buffer[1024];
	sprintf (buffer, "LIBRARIAN: Send BOOKS[");
	char* bufferPtr = buffer+strlen(buffer);

	set* tempset = new_set (int_printer, int_compare);

	int n;
	int max = rand() % LIBRARIAN_MAXBOOKSRQST;
	for (n=0;n<max; n++)
	{
		long id = rand()%LIBRARY_MAXBOOKIDS;
		set_insertInto(tempset, (any)id);
		bufferPtr = buffer+strlen(buffer);
		sprintf(bufferPtr, " %d", id);
	}
	bufferPtr = buffer+strlen(buffer);
	sprintf(bufferPtr, " ]\n");

	printf(buffer);
	msg_client_send (lbrn->client, "BOOKS", tempset);

	while(!set_isempty(tempset)) set_choose_item(tempset);
	set_release(tempset);
}

void librarian_LOANS (librarian_t* lbrn)
{
	assert(lbrn != NULL);
	printf ("LIBRARIAN: Send LOANS\n");

	set* tempset = new_set (int_printer, int_compare);

	int n;
	int max = rand() % LIBRARIAN_MAXLOANSRQST;
	for (n=0;n<max; n++)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		long id = rand()%20; //TODO MAGIC NUMBER!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		set_insertInto(tempset, (any)id);
	}

	msg_client_send (lbrn->client, "LOANS", tempset);

	while(!set_isempty(tempset)) set_choose_item(tempset);
	set_release(tempset);
}

void* librarian_run (void* arg)
{
	assert(arg != NULL);

	librarian_t lbrn;
	lbrn.client = msg_client_create ((msg_queue_t*)arg);

	librarian_ADD (&lbrn);

	int action;
	while (!shutdown)
	{
		delay_allowing_shutdown (LIBRARIAN_DELAY);

		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0: librarian_BOOKS(&lbrn); break;
				case 1: librarian_LOANS(&lbrn); break;
			}
		}
	}

	msg_client_release (lbrn.client);
}
