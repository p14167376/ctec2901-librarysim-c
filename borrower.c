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
#include "set.h"

// Project Headers
#define TRACE_ON
#include "smalloc.h"
#include "trace.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "borrower.h"


#define BORROWER_DELAY   5
#define MSG_BUFFER_SIZE  128
#define TEXT_BUFFER_SIZE 32

typedef struct
{
	msg_client_t* client;
	char          msgBuffer[MSG_BUFFER_SIZE];
	set*          myBooks;
} borrower_t;

void int_printer(any x) {printf("%d",(long)x);}
int  int_compare(any x, any y)
{
	if (x < y) return -1;
	if (x > y) return  1;
	return  0;
}

/*
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
*/


void borrower_RQST(borrower_t* brwr)
{
	printf ("BORROWER: Send RQST\n");

	//TODO

	msg_client_send (brwr->client, "RQST");
}

void borrower_RTRN(borrower_t* brwr)
{
	printf ("BORROWER: Send RQST\n");

	//TODO

	msg_client_send (brwr->client, "RQST");
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
