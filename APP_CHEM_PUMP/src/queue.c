// queue.c

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file implements a queue for transmitting data to the modem

#include <stdint.h>
#include <string.h>
#include "queue.h"
#include "assert_app.h"
#include "rtos.h"

// Private Functions
static bool isQueueFull(queue_t* pQueue);

//**********************************************************************************************************************
// Q_init - Initialize the queue for use
//**********************************************************************************************************************
Q_status_t Q_init(queue_t* pQueue, void* pBuf, uint16_t length, uint16_t width)
{
    Q_status_t status = Q_SUCCESS;

    assert(pBuf != NULL);

    pQueue->buf = pBuf;
    pQueue->length = length;
    pQueue->width = width;
    pQueue->putIndex = 0;
    pQueue->getIndex = 0;

    pQueue->resourceId = RtosResourceReserveID();
    
    if (pQueue->resourceId == RTOS_INVALID_ID)
    {
        status = Q_ERROR;
    }

    return status;
}

//**********************************************************************************************************************
// Q_put - Add a record to the queue
//**********************************************************************************************************************
Q_status_t Q_put(queue_t* pQueue, void* pBuf)
{
    assert(pQueue->buf != NULL);

    (void)K_Resource_Get(pQueue->resourceId);

    // Return early if queue is full
	if (isQueueFull(pQueue))
    {
        (void)K_Resource_Release(pQueue->resourceId);
        return Q_FULL;
    }

    memcpy(&pQueue->buf[pQueue->putIndex * pQueue->width], pBuf, pQueue->width);

	// Increment/wrap the put index
	if (pQueue->putIndex < (pQueue->length - 1))
	{
		pQueue->putIndex++;
	}
	else
    {
        pQueue->putIndex = 0;   
    }

    (void)K_Resource_Release(pQueue->resourceId);

    return Q_SUCCESS; 
}

//**********************************************************************************************************************
// Q_get - Retrieve a record from the queue
//**********************************************************************************************************************
Q_status_t Q_get(queue_t* pQueue, void* pBuf)
{
    assert(pQueue->buf != NULL);

    (void)K_Resource_Get(pQueue->resourceId);

    // Return early if queue is empty
	if (pQueue->putIndex == pQueue->getIndex)
    {
        (void)K_Resource_Release(pQueue->resourceId);
        return Q_EMPTY;
    }

    memcpy(pBuf, &pQueue->buf[pQueue->getIndex * pQueue->width], pQueue->width);

	// Increment/wrap the get index
	if (pQueue->getIndex < (pQueue->length - 1))
	{
		pQueue->getIndex++;
	}
	else
	{
        pQueue->getIndex = 0;   
    }

    (void)K_Resource_Release(pQueue->resourceId);

    return Q_SUCCESS; 
}

//**********************************************************************************************************************
// Q_flush - Initialize the queue
//**********************************************************************************************************************
Q_status_t Q_flush(queue_t* pQueue)
{
    (void)K_Resource_Get(pQueue->resourceId);

    pQueue->putIndex = 0;
    pQueue->getIndex = 0;

    (void)K_Resource_Release(pQueue->resourceId);

    return Q_SUCCESS; 
}

//**********************************************************************************************************************
// isQueueFull - Check to see if the queue is full
//**********************************************************************************************************************
static bool isQueueFull(queue_t* pQueue)
{
    uint16_t nextPutIndex = pQueue->putIndex + 1;

    if (nextPutIndex >= pQueue->length)
    {
        nextPutIndex = 0;   
    }

    if (nextPutIndex == pQueue->getIndex)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
