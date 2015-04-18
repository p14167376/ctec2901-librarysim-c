//===========================================================================
// FILE: smalloc.h
//===========================================================================
// Macro for allocating memory and handling failure.
// Failure of allocating memory is so unlikely on a modern system we simply
// report the failure and exit.
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// These wrappers were initially to wrap up common functionality.
// This tidies and reduces the code and reduces the opportunity for errors.
//
// The intention is to eventually use these to provide memory leak detection.
//---------------------------------------------------------------------------
// ******************************* WARNING *******************************
//   This macro declares a new variable - do not use existing variables.
//                    (Must use SAFE_MALLOC_EXISTING)
// ******************************* WARNING *******************************

#ifndef SMALLOC_H
#define SMALLOC_H

#include <malloc.h>

#define SAFE_MALLOC_EXISTING(t,x)                                \
	assert(x==NULL);                                             \
	x = (t*)malloc (sizeof(t));                                  \
	if (x == NULL)                                               \
	{                                                            \
		fprintf (stderr, "ERROR: malloc() failed in %s, %s()\n", \
			__FILE__, __func__, __LINE__);                       \
		exit(1);                                                 \
	}

#define SAFE_MALLOC(t,x)                                         \
	t* x = (t*)malloc (sizeof(t));                               \
	if (x == NULL)                                               \
	{                                                            \
		fprintf (stderr, "ERROR: malloc() failed in %s, %s()\n", \
			__FILE__, __func__, __LINE__);                       \
		exit(1);                                                 \
	}

#define SAFE_MALLOC_ARRAY(t,x,n)                                 \
	t* x = (t*)malloc (sizeof(t)*n);                             \
	if (x == NULL)                                               \
	{                                                            \
		fprintf (stderr, "ERROR: malloc() failed in %s, %s()\n", \
			__FILE__, __func__, __LINE__);                       \
		exit(1);                                                 \
	}

#define SAFE_FREE(t) free(t)

#endif//SMALLOC_H