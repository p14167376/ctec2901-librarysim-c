//===========================================================================
// FILE: smalloc.h
//===========================================================================
// Macro for allocating memory and handling failure.
// Failure of allocating memory is so unlikely on a modern system we simply
// report the failure and exit.
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef SMALLOC_H
#define SMALLOC_H

#include <malloc.h>

#define SAFE_MALLOC(t,x)                                         \
	t* x = (t*)malloc (sizeof(t));                               \
	if (x == NULL)                                               \
	{                                                            \
		fprintf (stderr, "ERROR: malloc() failed in %s, %s()\n", \
			__FILE__, __func__, __LINE__);                       \
	}

#endif//SMALLOC_H