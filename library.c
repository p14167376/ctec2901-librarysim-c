//===========================================================================
// FILE: library.c
//===========================================================================
// Implementation file for library code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------


// Standard C Headers
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Standard CTEC2901 Headers
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


avl_any* library_books;
msg_queue_t* lib_msg_queue = NULL;


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
	free (book);
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

void library_initialise()
{
	lib_msg_queue = new_msg_queue();
	library_books = new_avl_any (book_lessthan);
}

void library_cleanup()
{
	// Use the print mapping to free memory...
	avl_any_inorder_print (library_books, book_free);
	avl_any_release (library_books);
	msg_queue_release (lib_msg_queue);
}

book_t* library_findbook (int bookid)
{
	book_t dummybook;
	dummybook.id = bookid;
	return (book_t*)(avl_any_find (library_books, (any)&dummybook));
}

void library_addbook (int bookid)
{
	printf ("library_addbook(%d)\n", bookid);
	book_t* book = library_findbook (bookid);
	if (book == NULL)
	{
		printf ("--library_addbook::book_not_found\n");
		avl_any_insert (library_books, (any)book_create (bookid));
	}
	else
	{
		printf ("--library_addbook::book_found\n");
		book->copies++;
	}
}

void* library_thread (void* thread_id)
{
	// Create book structure
	library_initialise();

	int n;
	for (n=0; n<20; n++)
	{
		library_addbook (n);
	}

	avl_any_inorder_print (library_books, book_print);


	while (!shutdown)
	{
		msg_request_t* rqst = msg_queue_getrequest (lib_msg_queue);
		if (rqst != NULL)
		{
			// Process request...

			//msg_request_ack (rqst);
			msg_request_ack (rqst);
		}
	}

	library_cleanup();
}

main()
{
	// Read command line arguments...
	// TODO
	//library_thread(0);

    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

    int i;
    int error;
	int numBorrowers = 0;
    int numThreads = 2 + numBorrowers;
    SAFE_MALLOC_ARRAY(pthread_t, threads, numThreads);

    pthread_create(&threads[0], &attr, library_thread, (void*)0);
    //pthread_create(&threads[1], &attr, librarian_thread, (void*)1);
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
		}
		// nudge waiting processes...
		msg_queue_nudge(lib_msg_queue);
    }

    TRACE("Waiting for threads to close");
    for (i=0; i<numThreads; i++)
    {
      pthread_join(threads[i], NULL);
    }
  
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);

    TRACE("Program Complete");
    return 0;
}

