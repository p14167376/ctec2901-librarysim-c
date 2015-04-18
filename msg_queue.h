//===========================================================================
// FILE: msg-queue.h
//===========================================================================
// Header file for message queue functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include "queue_any.h"

typedef struct msg_queue_impl   msg_queue_t;
typedef struct msg_request_impl msg_request_t;

msg_queue_t*   new_msg_queue();
msg_request_t* new_msg_request      (msg_queue_t* msgq);

int            msg_request_send     (msg_request_t* rqst, any payload);
void           msg_queue_nudge      (msg_queue_t*   msgq);
msg_request_t* msg_queue_getrequest (msg_queue_t*   msgq);
void           msg_request_ack      (msg_request_t* rqst);

void           msg_queue_release    (msg_queue_t*   msgq);
void           msg_request_release  (msg_request_t* rqst);

#endif//MSG_QUEUE_H