// FSM.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// GraniteRiver, Minneapolis, MN
// All Rights Reserved

// This file runs the state machine for the pump control task

#include "FSM.h"

static const evt_t kEntryEvt = { SIG_ENTRY, EVT_NO_PAYLOAD };
static const evt_t kExitEvt = { SIG_EXIT, EVT_NO_PAYLOAD };
static const evt_t kInitEvt = { SIG_INIT, EVT_NO_PAYLOAD };

void FSM_init(fsm_t* pMe, handler_t initialState)
{
    pMe->state = initialState;

    FSM_dispatchEvt(pMe, &kInitEvt);
    FSM_dispatchEvt(pMe, &kEntryEvt);
}

void FSM_dispatchEvt(fsm_t* pMe, evt_t const *pEvt)
{
    handler_t stateHandler = pMe->state;
    state_t returnState = (*stateHandler)(pMe, pEvt);

    if (returnState == FSM_RET_TRAN)
    {
        (void)((*stateHandler)(pMe, &kExitEvt));
        (void)((*pMe->state)(pMe, &kEntryEvt));
    }
}

void FSM_dispatchSig(fsm_t* pMe, signal_t sig)
{
    evt_t event;

    event.sig = sig;
    event.dynamic = EVT_NO_PAYLOAD;

    FSM_dispatchEvt(pMe, &event);
}
