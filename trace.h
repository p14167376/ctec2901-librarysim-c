//===========================================================================
// FILE: trace.h
//===========================================================================
// Macros for debugging
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef TRACE_H
#define TRACE_H


#define TRACE_ON // turn this off for production!!


#ifdef TRACE_ON
#define TRACE(...) fprintf (stderr, "TRACE(%s,%s():%d): %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define TRACE(...)
#endif


#endif//TRACE_H