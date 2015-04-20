//===========================================================================
// FILE: library.h
//===========================================================================
// Header file for library code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef LIBRARY_H
#define LIBRARY_H


#include "set.h"
#include "msg_queue.h"


// TODO Make this a command line argument
#define LIBRARY_MAXBOOKIDS 5 //20

typedef struct
{
	int  brwr;
	set* books;
} library_RQST_t;

typedef struct library_impl library_t;

library_t*   library_create   (int numBorrowers);
void         library_release  (library_t* lib);
void*        library_run      (void* arg);
void         library_nudge    (library_t* lib);
msg_queue_t* library_getqueue (library_t* lib);


#endif//LIBRARY_H