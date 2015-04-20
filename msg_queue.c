//===========================================================================
// FILE: msg-queue.c
//===========================================================================
// Implementation file for message queue functionality
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// There are two structures used here:
//  The msg_queue_t is effectively the server, listening for messages from
//  clients which instantiate a msg_client_t structure.
//  The msg_client_t which handles sending requests to the queue.
//--------------------------------------------------------------------------- 


#include <assert.h>
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
	char* msgName;
	any   payload;
};

msg_queue_t* msg_queue_create()
{
	SAFE_MALLOC(msg_queue_t, msgq);
    pthread_mutex_init (&msgq->mutex, NULL);
    pthread_cond_init (&msgq->msg_waiting, NULL);
    msgq->messages = new_unbounded_queue_any();
    return msgq;
}

void msg_queue_ackall(msg_queue_t* msgq)
{
	assert(msgq != NULL);

	// Ack and remove any waiting messages...
	// (must ack to wake any waiting client threads)
	msg_client_t* client;
    while (!queue_any_isempty(msgq->messages))
	{
		client = (msg_client_t*)queue_any_dequeue(msgq->messages);
		put_mvar(client->ack, 0);
	}
}

void msg_queue_release(msg_queue_t* msgq)
{
	assert(msgq != NULL);

	msg_queue_ackall (msgq);
    queue_any_release (msgq->messages);
    pthread_cond_destroy(&msgq->msg_waiting);
    pthread_mutex_destroy(&msgq->mutex);
	SAFE_FREE(msgq);
}

msg_client_t* msg_client_create (msg_queue_t* msgq)
{
	assert(msgq != NULL);

	SAFE_MALLOC(msg_client_t, client);
	client->msgq = msgq;
    client->ack  = new_empty_mvar();
    // msgName & payload are set each time the client sends a request
    return client;
}

void msg_client_release (msg_client_t* client)
{
	assert(client != NULL);

	delete_mvar (client->ack);
	SAFE_FREE(client);
}

int msg_client_send (msg_client_t* client, char* msgName, any payload)
{
	assert(client != NULL);
	assert(msgName != NULL);
	// don't assert payload - could be NULL if we had a simple message

	// get a lock on the queue
    pthread_mutex_lock(&(client->msgq->mutex));
    if (shutdown)
    {
        pthread_mutex_unlock(&(client->msgq->mutex));
        return 0;
	}

	// add request to queue, tell the receiver and then free the queue
	client->msgName = msgName;
	client->payload = payload;
	queue_any_enqueue (client->msgq->messages, (any)client);
	pthread_cond_broadcast (&client->msgq->msg_waiting);
	pthread_mutex_unlock (&client->msgq->mutex);

	// wait for ack
	if (take_mvar (client->ack) != NULL) return 1;
	return 0;
}

char* msg_client_getmsgname (msg_client_t* client)
{
	return client->msgName;
}

any msg_client_getpayload (msg_client_t* client)
{
	return client->payload;
}

void msg_client_ack (msg_client_t* client)
{
	put_mvar(client->ack, (void*)0);//(client->payload));
}

void msg_queue_nudge (msg_queue_t* msgq)
{
	pthread_cond_broadcast (&msgq->msg_waiting);
	msg_queue_ackall(msgq);
}

msg_client_t* msg_queue_getclient (msg_queue_t* msgq)
{
	// gets the next message/client from the queue
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

