// FSM.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// GraniteRiver, Minneapolis, MN
// All Rights Reserved

// This file runs the state machine for the pump control task

#ifndef __FSM_H__
#define __FSM_H__

#include "stdint.h"

typedef uint8_t signal_t;

typedef struct
{
    signal_t sig;
    uint8_t dynamic;
} evt_t;

typedef uint8_t state_t;
typedef state_t (*handler_t) (void *pMe, evt_t const *pEvt);

typedef struct
{
    handler_t state;
} fsm_t;

void FSM_init(fsm_t* pMe, handler_t handler);
void FSM_dispatchEvt(fsm_t* pMe, evt_t const *pEvt);
void FSM_dispatchSig(fsm_t* pMe, signal_t sig);

#define FSM_RET_HANDLED     ((state_t)0)
#define FSM_RET_IGNORED     ((state_t)1)
#define FSM_RET_TRAN        ((state_t)2)
#define FSM_Handled()       (FSM_RET_HANDLED)
#define FSM_Ignored()       (FSM_RET_IGNORED)
#define FSM_TRAN(target)    (pMe->state = (handler_t)(target), FSM_RET_TRAN)

#define EVT_NO_PAYLOAD      (0)

enum reservedSignals
{
    SIG_ENTRY = 1,
    SIG_EXIT,
    SIG_INIT,
    SIG_USER
};

#endif
