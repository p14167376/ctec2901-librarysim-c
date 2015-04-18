//===========================================================================
// FILE: msg-queue.c
//===========================================================================
// Implementation file for message queue functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------

#include <pthread.h>
#include "trace.h"
#include "smalloc.h"
#include "shutdown.h"
#include "msg_queue.h"

typedef struct
{
	pthread_mutex_t mutex;
	pthread_cond_t  msg_waiting;
	queue_any*      messages;
} msg_queue_t;

typedef struct
{
	msg_queue_t* msgq;
	mvar* reply;
	any payload;
} msg_request_t;

msg_queue_t* new_msg_queue()
{
	SAFE_MALLOC(msg_queue_t, newqueue);
    pthread_mutex_init (&newqueue->mutex, NULL);
    pthread_cond_init (&newqueue->msg_waiting, NULL);
    return newqueue;
}

msg_request_t* new_msg_request (msg_queue_t* msgq)
{
	SAFE_MALLOC(msg_request_t, rqst);
	rqst->msgq  = msgq;
    rqst->reply = new_empty_mvar();
    // payload is set each time the owner sends a request
}

int msg_request_send (msg_request_t* rqst, any payload)
{        
    pthread_mutex_lock(&(rqst->msgq->mutex));
    if (shutdown)
    {
        pthread_mutex_unlock(&(rqst->msgq->mutex));
        return false;
	}

	rqst->payload = payload;
	queue_any_enqueue (rqst->msgq->messages, (any)(&rqst));

	pthread_cond_broadcast (&rqst->msgq->msg_waiting);
	pthread_mutex_unlock (&rqst->msgq->mutex);

	take_mvar (rqst->reply);
	return true;
}

msg_request* msg_queue_getrequest (msg_queue_t* msgq)
{
	pthread_mutex_lock(&msgq->mutex);

	while (queue_any_isempty(msgq->messages))
	{
	    pthread_cond_wait(&msgq->msg_waiting, &msgq->mutex);
	}

	msg_request_t* rqst = (msg_request_t*)(queue_any_dequeue(msgq->messages));
	pthread_mutex_unlock(&msgq->mutex);
	return rqst;
}

void msq_request_reply (msg_request_t* rqst)
{
	put_mvar(rqst->reply, (void*)(rqst->payload));
}
