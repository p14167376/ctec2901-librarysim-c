//===========================================================================
// FILE: librarian.h
//===========================================================================
// Header file for librarian code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef LIBRARIAN_H
#define LIBRARIAN_H

typedef struct librarian_impl librarian_t;

librarian_t* librarian_create  (msg_queue_t* msgq);
void         librarian_release (librarian_t* lbrn);

#endif//LIBRARIAN_H