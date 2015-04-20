//===========================================================================
// FILE: library.c
//===========================================================================
// Implementation file for library code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// Notes
//  Rather than expose functions for getting number of borrowers and
//  registering borrowers, I have opted to use messages so that we are
//  only implementing one type of interface and not multiple types.
//---------------------------------------------------------------------------



// Standard C Headers
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// ds library Headers
#include "list.h"
#include "clist.h"
#include "queue_any.h"

// Project Headers
#define TRACE_ON
#include "smalloc.h"
#include "trace.h"
#include "terminal.h"
#include "mvar.h"
#include "set_ints.h"
#include "avl_any.h"
#include "shutdown.h"
#include "msg_queue.h"
#include "library.h"
#include "librarian.h"
#include "borrower.h"



#define SAFE_CREATE_THREAD(t,a,f,d)                                    \
	{                                                                  \
        int error = pthread_create(t,a,f,d);                           \
        if (error != 0)                                                \
        {                                                              \
            printf("ERROR: pthread_create() failed, code %i\n",error); \
            exit(1);                                                   \
        }                                                              \
    }

struct library_impl
{
	avl_any*     books;
	msg_queue_t* msgQueue;
	int          numBorrowers;
	int          nextId;
};

typedef struct book_struct
{
	int  id;
	int  copies;
	set* borrowerSet;
} book_t;

book_t* book_create (int id)
{
	SAFE_MALLOC(book_t, book);
	book->id = id;
	book->copies = 1;
	book->borrowerSet = set_ints_create();
	return book;
}

void book_free (any x)
{
	assert(x != NULL);

	book_t* book = (book_t*)x;
	set_ints_release(book->borrowerSet);
	SAFE_FREE(book);
}

int book_lessthan (any x, any y)
{
	assert(x != NULL);
	assert(y != NULL);
	return (int)(((book_t*)x)->id < ((book_t*)y)->id);
}

library_t* library_create (int numBorrowers)
{
	SAFE_MALLOC(library_t,lib);
	lib->msgQueue     = msg_queue_create();
	lib->books        = new_avl_any (book_lessthan);
	lib->numBorrowers = numBorrowers;
	lib->nextId       = 0;
	return lib;
}

void library_release (library_t* lib)
{
	assert(lib != NULL);

	// Use the print mapping to free memory...
	avl_any_inorder_print (lib->books, book_free);
	avl_any_release (lib->books);
	msg_queue_release (lib->msgQueue);
}

void library_nudge (library_t* lib)
{
	assert(lib != NULL);
	msg_queue_nudge(lib->msgQueue);
}

msg_queue_t* library_getqueue (library_t* lib)
{
	assert(lib != NULL);
	return lib->msgQueue;
}

book_t* library_findbook (library_t* lib, int bookid)
{
	assert(lib != NULL);
	assert(bookid >= 0);

	book_t dummybook;
	dummybook.id = bookid;
	return (book_t*)(avl_any_find (lib->books, (any)&dummybook));
}

void library_addbook (library_t* lib, int bookid)
{
	assert(lib != NULL);
	assert(bookid >= 0);

	book_t* book = library_findbook (lib, bookid);
	if (book == NULL)
	{
		avl_any_insert (lib->books, (any)book_create (bookid));
	}
	else
	{
		book->copies++;
	}
}

void library_printbook (any x)
{
	assert(x != NULL);

	book_t* book = (book_t*)x;
	printf ("Book %03d: Owned(%d), Loaned(%d)",
		book->id, book->copies, set_count (book->borrowerSet));
	if (set_count (book->borrowerSet))
	{
		printf (" Borrower(s)");
		set_print (book->borrowerSet);
	}
	printf ("\n");
}

typedef struct gbfb_context_impl
{
	int  brwr;
	set* books;
} gbfb_context_t;

void library_checkbookforborrower (any x, any c)
{
	assert(x != NULL);
	assert(c != NULL);

	book_t* book = (book_t*)x;
	gbfb_context_t* context = (gbfb_context_t*)c;

	if (set_ints_isin(book->borrowerSet, context->brwr))
	{
		set_ints_insertinto (context->books, book->id);
	}
}

set* library_getbooksforborrower (library_t* lib, int brwr)
{
	assert(lib != NULL);
	assert(brwr >= 0);

	gbfb_context_t context;
	context.brwr  = brwr;
	context.books = set_ints_create();
	avl_any_inorder_map(lib->books,
		library_checkbookforborrower, (any)(&context));
	return context.books;
}

void library_printborrower (library_t* lib, int brwr)
{
	assert(lib != NULL);
	assert(brwr >= 0);

	set* books = library_getbooksforborrower(lib, brwr);
	if ( (books)
	&&   (set_count(books) > 0) )
	{
		printf("Borrower %d: Has Books", brwr);
		set_print (books);
		printf("\n", brwr);
	}
	else
	{
		printf("Borrower %d: Has no books\n", brwr);
	}
	set_ints_release(books);
}

void library_GETNB(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);

	int* id = (int*)payload;
	*id = lib->numBorrowers;
}

void library_ADD(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);

	list* newbooks = (list*)payload;
	printf ("MSG(ADD): Adding %d new books to the library\n",
		list_size(newbooks));

    list_goto_head(newbooks);
    while (list_cursor_inlist(newbooks))
    {
		library_addbook(lib, (long)list_get_item(newbooks));
        list_goto_next(newbooks);
    }
}

void library_BOOKS(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);
	terminal_settextcyan(1);
	printf ("MSG(BOOKS): Displaying status of selected books - - - - - - - - - - - - - - -\n");

	set* tempset = (set*)payload;
	set* copyset = set_ints_create();
	set_unionWith(copyset, tempset);

	while(!set_isempty(copyset))
	{
		book_t* book = library_findbook(lib, (long)set_choose_item(copyset));
		if (book) library_printbook((any)book);
	}
	set_ints_release(copyset);
	printf ("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	terminal_reset();
}

void library_LOANS(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);
	terminal_settextgreen(1);
	printf ("MSG(LOANS): Displaying status of selected borrowers - - - - - - - - - - - - -\n");

	set* tempset = (set*)payload;
	set* copyset = set_ints_create();
	set_unionWith(copyset, tempset);

	while(!set_isempty(copyset))
	{
		long id = (long)set_choose_item(copyset);
		library_printborrower(lib, id);
	}
	set_ints_release(copyset);
	printf ("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	terminal_reset();
}

void library_RGST(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);

	int* id = (int*)payload;
	*id = lib->nextId++;
	//printf ("MSG(RGST): Registered borrower (id %d)\n", *id);
}

void library_RQST(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);

	library_RQST_t* librq = (library_RQST_t*)payload;
	set* copyset = set_ints_create();
	set_unionWith(copyset, librq->books);

	terminal_settextyellow(1);
	printf ("MSG(RQST): Borrower %02d, Requests Books", librq->brwr);
	set_print (librq->books);

	while(!set_isempty(copyset))
	{
		int bookid = (long)set_choose_item(copyset);
		book_t* book = library_findbook(lib, bookid);
		if ( (book)
		&&   (!set_ints_isin(book->borrowerSet, librq->brwr))
		&&   (set_count(book->borrowerSet) < book->copies) )  
		{
			set_ints_insertinto(book->borrowerSet, librq->brwr);
		}
		else set_ints_removefrom (librq->books, bookid);
	}
	set_ints_release(copyset);

	printf (", Got");
	set_print (librq->books);
	printf("\n");
	terminal_reset();
}

void library_RTRN(library_t* lib, any payload)
{
	assert(lib != NULL);
	assert(payload != NULL);

	library_RQST_t* librq = (library_RQST_t*)payload;
	set* copyset = set_ints_create();
	set_unionWith(copyset, librq->books);

	terminal_settextyellow(0);
	printf ("MSG(RTRN): Borrower %02d Returns Books", librq->brwr);
	set_print (librq->books);
	printf("\n");

	while(!set_isempty(copyset))
	{
		int bookid = (long)set_choose_item(copyset);
		book_t* book = library_findbook(lib, bookid);
		if ( (book)
		&&   (set_ints_isin(book->borrowerSet, librq->brwr)) )
		{
			set_ints_removefrom(book->borrowerSet, librq->brwr);
		}
	}
	set_ints_release(copyset);
	terminal_reset();
}

void* library_run (void* arg)
{
	assert(arg != NULL);
	library_t* lib = (library_t*)arg;

	while (!shutdown)
	{
		msg_client_t* client = msg_queue_getclient (lib->msgQueue);
		if (client != NULL)
		{
			// Process request from client...
			char* msgName = msg_client_getmsgname(client);
			any   pl      = msg_client_getpayload(client);

			if      (strncmp(msgName, "GETNB", 5)==0) library_GETNB (lib, pl);
			else if (strncmp(msgName, "ADD",   3)==0) library_ADD   (lib, pl);
			else if (strncmp(msgName, "BOOKS", 5)==0) library_BOOKS (lib, pl);
			else if (strncmp(msgName, "LOANS", 5)==0) library_LOANS (lib, pl);
			else if (strncmp(msgName, "RGST",  4)==0) library_RGST  (lib, pl);
			else if (strncmp(msgName, "RQST",  4)==0) library_RQST  (lib, pl);
			else if (strncmp(msgName, "RTRN",  4)==0) library_RTRN  (lib, pl);
			else
			{
				printf("Unrecognised Message Received: '%s'\n", msgName);
			}

			msg_client_ack (client);
		}
	}
}
