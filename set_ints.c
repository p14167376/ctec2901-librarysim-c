//===========================================================================
// FILE: set_ints.c
//===========================================================================
// Wrapper functions for handling sets of ints.
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "set.h"
#include "set_ints.h"


set* set_ints_create()
{
	return new_set (set_ints_printer, set_ints_compare);
}

int  set_ints_isin(set* s, int n)
{
	assert(s != NULL);
	return set_isin (s,(any)(long)n);
}

void set_ints_insertinto(set* s, int n)
{
	assert(s != NULL);
	set_insertInto (s,(any)(long)n);
}

void set_ints_removefrom(set* s, int n)
{
	assert(s != NULL);
	set_removeFrom (s,(any)(long)n);
}

void set_ints_removeall(set* s)
{
	assert(s != NULL);
	while(!set_isempty(s)) set_choose_item(s);
}

void set_ints_release(set* s)
{
	assert(s != NULL);
	set_ints_removeall(s);
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
