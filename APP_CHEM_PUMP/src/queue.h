// queue.h

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for the queue used to transmit data to the modem

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>

typedef struct
{   
    uint8_t*            buf;
    uint16_t            length;
    uint16_t            width;
    uint16_t            putIndex;
    uint16_t            getIndex;
    uint8_t             resourceId;
} queue_t;

typedef enum
{
    Q_SUCCESS = 0,
    Q_FULL,
    Q_EMPTY,
    Q_ERROR
} Q_status_t;

Q_status_t Q_init(queue_t* pQueue, void* pBuf, uint16_t length, uint16_t width);
Q_status_t Q_put(queue_t* pQueue, void* pBuf);
Q_status_t Q_get(queue_t* pQueue, void* pBuf);
Q_status_t Q_flush(queue_t* pQueue);

#endif
