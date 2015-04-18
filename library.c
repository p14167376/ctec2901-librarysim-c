//===========================================================================
// FILE: library.c
//===========================================================================
// Implementation file for library code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------


// Standard C Headers
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// ds library Headers
#include "clist.h"
#include "queue_any.h"

// Project Headers
#define TRACE_ON
#include "smalloc.h"
#include "trace.h"
#include "mvar.h"
#include "avl_any.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "library.h"
#include "librarian.h"


typedef struct
{
	avl_any* books;
	msg_queue_t* msg_queue;
} library_t;


typedef struct book_struct
{
	int    id;
	int    copies;
	clist* borrowerlist;
} book_t;

book_t* book_create (int id)
{
	SAFE_MALLOC(book_t, book);
	book->id = id;
	book->copies = 1;
	book->borrowerlist = new_clist();
	return book;
}

void book_free (any x)
{
	book_t* book = (book_t*)x;
	clist_release (book->borrowerlist);
	SAFE_FREE(book);
}

int book_lessthan (any x, any y)
{
	return (int)(((book_t*)x)->id < ((book_t*)y)->id);
}

void book_print_borrower (any x)
{
	printf (" %d", (long)x);
}

void book_print (any x)
{
	book_t* book = (book_t*)x;
	printf ("Book %03d: %d Copies Owned; %d Copies on loan.", book->id, book->copies, clist_size (book->borrowerlist));
	if (clist_size (book->borrowerlist))
	{
		printf (" Borrower(s) ");
		clist_print (book->borrowerlist, book_print_borrower);
	}
	printf ("\n");
}

library_t* library_create()
{
	SAFE_MALLOC(library_t,lib);
	lib->msg_queue = msg_queue_create();
	lib->books = new_avl_any (book_lessthan);
	return lib;
}

void library_release (library_t* lib)
{
	// Use the print mapping to free memory...
	avl_any_inorder_print (lib->books, book_free);
	avl_any_release (lib->books);
	msg_queue_release (lib->msg_queue);
}

book_t* library_findbook (library_t* lib, int bookid)
{
	printf ("library_findbook(%d)\n", bookid);
	book_t dummybook;
	dummybook.id = bookid;
	return (book_t*)(avl_any_find (lib->books, (any)&dummybook));
}

void library_addbook (library_t* lib, int bookid)
{
	printf ("library_addbook(%d)\n", bookid);
	book_t* book = library_findbook (lib, bookid);
	if (book == NULL)
	{
		printf ("--library_addbook::book_not_found\n");
		avl_any_insert (lib->books, (any)book_create (bookid));
	}
	else
	{
		printf ("--library_addbook::book_found\n");
		book->copies++;
	}
}

void library_ADD(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);

	// TODO Add the list of books to the library

	/*
	if (strncmp(msgText, "ADD(", 4) != 0)
	{
		printf ("LIBRARY: Badly formatted ADD command.\n");
		return;
	}
	msgText += 4; // jump past "ADD("

	int scan;
	int bookid;
	while (*msgText != 0)
	{
		scan = sscanf (msgText, "%d", &bookid);
		if (scan == 1)
		{
			library_addbook(lib, bookid);
			while (*msgText != ',' && *msgText != 0) msgText++;
		}
		if (*msgText != 0) msgText++;
	}
	*/
}

void library_BOOKS(library_t* lib, any payload)
{
	assert(lib != NULL);
	avl_any_inorder_print (lib->books, book_print);
}

void library_LOANS(library_t* lib, any payload)
{
	assert(lib != NULL);

}

void library_RQST(library_t* lib, any payload)
{
	assert(lib != NULL);

}

void library_RTRN(library_t* lib, any payload)
{
	assert(lib != NULL);

}

void* library_run (void* arg)
{
	assert(arg != NULL);
	library_t* lib = (library_t*)arg;

	while (!shutdown)
	{
		msg_client_t* client = msg_queue_getclient (lib->msg_queue);
		if (client != NULL)
		{
			// Process request from client...
			char* msgName = msg_client_getmsgname(client);
			any   payload = msg_client_getpayload(client);

			if      (strncmp(msgName, "ADD",   3) == 0) library_ADD   (lib, payload);
			else if (strncmp(msgName, "BOOKS", 5) == 0) library_BOOKS (lib, payload);
			else if (strncmp(msgName, "LOANS", 5) == 0) library_LOANS (lib, payload);
			else if (strncmp(msgName, "RQST",  4) == 0) library_RQST  (lib, payload);
			else if (strncmp(msgName, "RTRN",  4) == 0) library_RTRN  (lib, payload);
			else
			{
				printf("Unrecognised Message Received: '%s'\n", msgName);
			}

			msg_client_ack (client);
		}
	}
}

main()
{
	// Read command line arguments...
	// TODO
	printf("\n\n");
	printf("===============================================================================\n");
	printf("CTEC2901: Library Simulator                                   (Barnaby Stewart)\n");
	printf("===============================================================================\n");

	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

	int i;
	int error;
	int numBorrowers = 0;
	int numThreads = 2 + numBorrowers;
	SAFE_MALLOC_ARRAY(pthread_t, threads, numThreads);

	library_t* lib = library_create();
	pthread_create(&threads[0], &attr, library_run,   (void*)lib);
	pthread_create(&threads[1], &attr, librarian_run, (void*)lib->msg_queue);

    /*
    // Create threads for borrowers
    for (i=2; i<numThreads; i++)
    {
        error = pthread_create(&threads[i], &attr, sender, (void*)i);
        if (err!=0)
        {
            printf("Create failed at %i\n",i);
            exit(1);
        }
    }
    */

    char inputBuffer[256];
    while (!shutdown)
    {
    	printf ("Enter command ('q' to quit): ");
		fgets (inputBuffer, sizeof(inputBuffer), stdin);
		switch (inputBuffer[0])
		{
			case 'q':
			case 'Q':
				shutdown = 1;
				break;
		}
		// nudge waiting processes...
		msg_queue_nudge(lib->msg_queue);
    }

	TRACE("Waiting for threads to close");
	//for (i=0; i<numThreads; i++)
	for (i=0; i<2; i++)
	{
		pthread_join(threads[i], NULL);
	}
	library_release (lib);

	pthread_attr_destroy(&attr);

	printf("-------------------------------------------------------------------------------\n");
	printf("Program Complete\n");
	printf("-------------------------------------------------------------------------------\n");
	return 0;
}

