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

// Project Headers
#include "smalloc.h"
#include "trace.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "librarian.h"
#include "library.h"


#define LIBRARIAN_DELAY  5
#define MSG_BUFFER_SIZE  128
#define TEXT_BUFFER_SIZE 32

typedef struct
{
	msg_client_t* client;
	char          msgBuffer[MSG_BUFFER_SIZE];
} librarian_t;

//typedef enum {ACTION_BOOKS_REQUEST, ACTION_LOANS_REQUEST} action_t;

void generate_book_msg (char* msgBuffer)
{
	assert(msgBuffer != NULL);

	// Add books to the library...
	int n;
	int firstBook = 1;
	char textBuffer[TEXT_BUFFER_SIZE];
	sprintf(msgBuffer, "ADD(");
	srand(time(NULL));
	for (n=0; n<20; n++)
	{
		int id = rand()%LIBRARY_MAXBOOKIDS;
		if (firstBook)
		{
			firstBook=0;
			sprintf(textBuffer, "%d", id);
		}
		else
		{
			sprintf(textBuffer, ",%d", id);
		}
		strcat (msgBuffer, textBuffer);
	}
	strcat (msgBuffer, ")");
}

void generate_random_books (librarian_t* lbrn)
{
	assert(lbrn != NULL);

	generate_book_msg (lbrn->msgBuffer);
	msg_client_send (lbrn->client, lbrn->msgBuffer);

	generate_book_msg (lbrn->msgBuffer);
	msg_client_send (lbrn->client, lbrn->msgBuffer);

	generate_book_msg (lbrn->msgBuffer);
	msg_client_send (lbrn->client, lbrn->msgBuffer);
}

void librarian_simplemsg (librarian_t* lbrn, char* msgText)
{
	assert(lbrn != NULL);
	assert(msgText != NULL);

	//printf ("LIBRARIAN: Send %s request\n", msgText);
	msg_client_send (lbrn->client, msgText);
}

void* librarian_run (void* arg)
{
	assert(arg != NULL);

	librarian_t lbrn;
	lbrn.client = msg_client_create ((msg_queue_t*)arg);

	int action;
	char msgBuffer[MSG_BUFFER_SIZE];

	generate_random_books (&lbrn);

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
