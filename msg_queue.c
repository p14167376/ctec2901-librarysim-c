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

struct msg_client_impl
{
	msg_queue_t* msgq;
	mvar* ack;
	any payload;
};

msg_queue_t* msg_queue_create()
{
	SAFE_MALLOC(msg_queue_t, msgq);
    pthread_mutex_init (&msgq->mutex, NULL);
    pthread_cond_init (&msgq->msg_waiting, NULL);
    msgq->messages = new_unbounded_queue_any();
    return msgq;
}

void msg_queue_release(msg_queue_t* msgq)
{
	// Ack and remove any waiting messages...
	// (must ack to wake any waiting threads)
	msg_client_t* client;
    while (!queue_any_isempty(msgq->messages))
	{
		client = (msg_client_t*)queue_any_dequeue(msgq->messages);
		put_mvar(client->ack, 0);
	}
    queue_any_release (msgq->messages);
    pthread_cond_destroy(&msgq->msg_waiting);
    pthread_mutex_destroy(&msgq->mutex);
	SAFE_FREE(msgq);
}

msg_client_t* msg_client_create (msg_queue_t* msgq)
{
	SAFE_MALLOC(msg_client_t, client);
	client->msgq = msgq;
    client->ack  = new_empty_mvar();
    // payload is set each time the client sends a request
    return client;
}

void msg_client_release (msg_client_t* client)
{
	delete_mvar (client->ack);
	SAFE_FREE(client);
}

int msg_client_send (msg_client_t* client, any payload)
{        
    pthread_mutex_lock(&(client->msgq->mutex));
    if (shutdown)
    {
        pthread_mutex_unlock(&(client->msgq->mutex));
        return 0;
	}

	client->payload = payload;
	queue_any_enqueue (client->msgq->messages, (any)(&client));

	pthread_cond_broadcast (&client->msgq->msg_waiting);
	pthread_mutex_unlock (&client->msgq->mutex);

	if (take_mvar (client->ack) != NULL) return 1;
	return 0;
}

any msg_client_getpayload (msg_client_t* client)
{
	return client->payload;
}

void msg_client_ack (msg_client_t* client)
{
	put_mvar(client->ack, (void*)(client->payload));
}

void msg_queue_nudge (msg_queue_t* msgq)
{
	pthread_cond_broadcast (&msgq->msg_waiting);
}

msg_client_t* msg_queue_getclient (msg_queue_t* msgq)
{
	pthread_mutex_lock(&msgq->mutex);

	while (queue_any_isempty(msgq->messages))
	{
	    pthread_cond_wait(&msgq->msg_waiting, &msgq->mutex);
		if (shutdown) return NULL;
	}
	msg_client_t* client = (msg_client_t*)(queue_any_dequeue(msgq->messages));
	pthread_mutex_unlock(&msgq->mutex);
	return client;
}

