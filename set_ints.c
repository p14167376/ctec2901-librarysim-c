//===========================================================================
// FILE: shutdown.c
//===========================================================================
// Implementation file for shutdown functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// Provides delay functions that respond to shutdown at intervals
// NOTE: these are not accurate as additional time stopping & starting.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "set.h"
#include "set_ints.h"

set* set_ints_create()
{
	return new_set (set_ints_printer, set_ints_compare);
}

void set_ints_release (set* s)
{
	while(!set_isempty(s)) set_choose_item(s);
	set_release (s);
}

void set_ints_printer(any x)
{
	printf("%d",(long)x);
}

int  set_ints_compare(any x, any y)
{
	if (x < y) return -1;
	if (x > y) return  1;
	return  0;
}
