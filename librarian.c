//===========================================================================
// FILE: librarian.c
//===========================================================================
// Implementation file for librarian code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------


// Standard C Headers
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


#define LIBRARIAN_DELAY  5
#define MSG_BUFFER_SIZE  128
#define TEXT_BUFFER_SIZE 32

//typedef enum {ACTION_BOOKS_REQUEST, ACTION_LOANS_REQUEST} action_t;

void generate_book_msg (char* msgBuffer)
{
	// Add books to the library...
	int n;
	int firstBook = 1;
	char textBuffer[TEXT_BUFFER_SIZE];
	sprintf(msgBuffer, "ADD(");
	srand(time(NULL));
	for (n=0; n<20; n++)
	{
		int id = rand()%20;
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

void* librarian_run (void* arg)
{
	int action;
	int delayLoop;
	char msgBuffer[MSG_BUFFER_SIZE];
	msg_client_t* client = msg_client_create ((msg_queue_t*)arg);

	generate_book_msg (msgBuffer);
	msg_client_send (client, msgBuffer);

	generate_book_msg (msgBuffer);
	msg_client_send (client, msgBuffer);

	generate_book_msg (msgBuffer);
	msg_client_send (client, msgBuffer);

	while (!shutdown)
	{
		for(delayLoop=0;delayLoop<LIBRARIAN_DELAY;delayLoop++)
		{
			if (!shutdown) sleep(1);
		}
		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0:
					printf ("LIBRARIAN: Send BOOK request\n");
					msg_client_send (client, "BOOKS");
					break;
				case 1:
					printf ("LIBRARIAN: Send LOANS request\n");
					msg_client_send (client, "LOANS");
					break;
			}
		}
	}

	msg_client_release (client);
}
