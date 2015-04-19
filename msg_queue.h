//===========================================================================
// FILE: msg-queue.h
//===========================================================================
// Header file for message queue functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include "queue_any.h"

typedef struct msg_queue_impl  msg_queue_t;
typedef struct msg_client_impl msg_client_t;

msg_queue_t*  msg_queue_create();
msg_client_t* msg_client_create      (msg_queue_t* msgq);

int    msg_client_send (msg_client_t* client, char* msgName, any payload);
void          msg_queue_nudge        (msg_queue_t*  msgq);
msg_client_t* msg_queue_getclient    (msg_queue_t*  msgq);
char*         msg_client_getmsgname  (msg_client_t* client);
any           msg_client_getpayload  (msg_client_t* client);
void          msg_client_ack         (msg_client_t* client);

void          msg_queue_release      (msg_queue_t*  msgq);
void          msg_client_release     (msg_client_t* client);

#endif//MSG_QUEUE_H