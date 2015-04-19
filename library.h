//===========================================================================
// FILE: library.h
//===========================================================================
// Header file for library code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef LIBRARY_H
#define LIBRARY_H


#define LIBRARY_MAXBOOKIDS 5 //20

typedef struct
{
	int  brwr;
	set* books;
} library_RQST_t;

#endif//LIBRARY_H