//===========================================================================
// FILE: msg-queue.c
//===========================================================================
// Implementation file for message queue functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------

#include <pthread.h>

#include "mvar.h"
#include "trace.h"
#include "smalloc.h"

#include "shutdown.h"
#include "msg_queue.h"

struct msg_queue_impl
{
	pthread_mutex_t mutex;
	pthread_cond_t  msg_waiting;
	queue_any*      messages;
};

struct msg_request_impl
{
	msg_queue_t* msgq;
	mvar* ack;
	any payload;
};

msg_queue_t* new_msg_queue()
{
	SAFE_MALLOC(msg_queue_t, msgq);
    pthread_mutex_init (&msgq->mutex, NULL);
    pthread_cond_init (&msgq->msg_waiting, NULL);
    msgq->messages = new_unbounded_queue_any();
    return msgq;
}

msg_request_t* new_msg_request (msg_queue_t* msgq)
{
	SAFE_MALLOC(msg_request_t, rqst);
	rqst->msgq = msgq;
    rqst->ack  = new_empty_mvar();
    // payload is set each time the owner sends a request
}

void msg_queue_release(msg_queue_t* msgq)
{
	// Ack and remove any waiting messages...
	// (must ack to wake any waiting threads)
	msg_request_t* rqst;
    while (!queue_any_isempty(msgq->messages))
	{
		rqst = (msg_request_t*)queue_any_dequeue(msgq->messages);
		put_mvar(rqst->ack, 0);
	}
    queue_any_release (msgq->messages);
    pthread_cond_destroy(&msgq->msg_waiting);
    pthread_mutex_destroy(&msgq->mutex);
	free (msgq);
}

void msg_request_release (msg_request_t* rqst)
{
	delete_mvar (rqst->ack);
	free (rqst);
}

int msg_request_send (msg_request_t* rqst, any payload)
{        
    pthread_mutex_lock(&(rqst->msgq->mutex));
    if (shutdown)
    {
        pthread_mutex_unlock(&(rqst->msgq->mutex));
        return 0;
	}

	rqst->payload = payload;
	queue_any_enqueue (rqst->msgq->messages, (any)(&rqst));

	pthread_cond_broadcast (&rqst->msgq->msg_waiting);
	pthread_mutex_unlock (&rqst->msgq->mutex);

	take_mvar (rqst->ack);
	return 1;
}

void msg_request_ack (msg_request_t* rqst)
{
	put_mvar(rqst->ack, (void*)(rqst->payload));
}

void msg_queue_nudge (msg_queue_t* msgq)
{
	pthread_cond_broadcast (&msgq->msg_waiting);
}

msg_request_t* msg_queue_getrequest (msg_queue_t* msgq)
{
	pthread_mutex_lock(&msgq->mutex);

	while (queue_any_isempty(msgq->messages))
	{
	    pthread_cond_wait(&msgq->msg_waiting, &msgq->mutex);
		if (shutdown) return NULL;
	}
	msg_request_t* rqst = (msg_request_t*)(queue_any_dequeue(msgq->messages));
	pthread_mutex_unlock(&msgq->mutex);
	return rqst;
}

