//===========================================================================
// FILE: set_ints.h
//===========================================================================
// Header file for set of ints functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef SETINTS_H
#define SETINTS_H

#include "set.h"

set* set_ints_create();
int  set_ints_isin       (set* s, int n);
void set_ints_insertinto (set* s, int n);
void set_ints_removefrom (set* s, int n);
void set_ints_removeall  (set* s);
void set_ints_release    (set* s);

void set_ints_printer(any x);
int  set_ints_compare(any x, any y);

#endif//SET_INTS_H