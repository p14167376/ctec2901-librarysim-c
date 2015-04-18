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
#include "library.h"


avl_any* library_books;
queue_any* lib_msg_queue = NULL;


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
	lib_msg_queue = new_unbounded_queue_any();
	library_books = new_avl_any (book_lessthan);
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

void library_deleteallbooks()
{
	// Use the print mapping to free memory...
	avl_any_inorder_print (library_books, book_free);
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

	printf ("avl_any_preorder_print()\n");
	avl_any_inorder_print (library_books, book_print);

	printf ("library_deleteallbooks()\n");
	library_deleteallbooks();
}

main()
{
	// Read command line arguments...
	// TODO
	library_thread(0);
/*

    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

    //pthread_t threads[NO_THREADS];
    for(i=1;i<NO_THREADS;i++)
    {
        err = pthread_create(&threads[i], &attr, sender, (void *)(thread_id[i]));
        if (err!=0)
        {
            printf("Create failed at %i\n",i);
            exit(1);
        }
    }
    pthread_create(&threads[0], &attr, receiver, (void *)(thread_id[0]));

    for (i=0; i<NO_THREADS; i++)
      pthread_join(threads[i], NULL);
  
    printf("Main done.\n");
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_cond_destroy(&msg_waiting_cond_var);
    pthread_exit(NULL);
    queue_any_release(buffer);
    */
    return 0;
}

