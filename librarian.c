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


#define LIBRARIAN_DELAY  1
#define MSG_BUFFER_SIZE  128
#define TEXT_BUFFER_SIZE 32

//typedef enum {ACTION_BOOKS_REQUEST, ACTION_LOANS_REQUEST} action_t;

void generate_book_msg (char* msgBuffer)
{
	// Add books to the library...
	int n;
	int firstBook = 1;
	char textBuffer[TEXT_BUFFER_SIZE];
	sprintf(msgBuffer, "ADDBOOKS(");
	srand(time(NULL));
	for (n=0; n<20; n++)
	{
		int id = rand()%1000;
		if (firstBook)
		{
			sprintf(textBuffer, "%d", id);
		}
		else
		{
			sprintf(textBuffer, ",%d", id);
			firstBook=0;
		}
		strcat (msgBuffer, textBuffer);
	}
	strcat (msgBuffer, ")");
}

void* librarian_run (void* arg)
{
	int action;
	char msgBuffer[MSG_BUFFER_SIZE];
	msg_client_t* client = msg_client_create ((msg_queue_t*)arg);

	generate_book_msg (msgBuffer);
	msg_client_send (client, msgBuffer);

	while (!shutdown)
	{
		sleep(LIBRARIAN_DELAY);
		if (!shutdown)
		{
			action = rand()%2;
			switch (action)
			{
				case 0:
					msg_client_send (client, "REQUEST_BOOKS");
					break;
				case 1:
					msg_client_send (client, "REQUEST_LOANS");
					break;
			}
		}
	}

	msg_client_release (client);
}
