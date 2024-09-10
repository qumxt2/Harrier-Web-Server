// event_handler.c

// Copyright 2010-2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved


// DESCRIPTION
// This file contains all the event handling code


#include "typedef.h"

#include "stdio.h"
#include "string.h"

#include "rtos.h"
#include "debug.h"

#include "dvar.h"

#include "event_defs.h"

#include "event_handler.h"


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************
typedef struct eventlist_element
{
	DistVarType eventCode;
	EventAckCallback_t ackCallback;
	EventClearFunction_t clearFunction;
	uint8 ackTimeout_seconds;
	struct eventlist_element *next;
} EVENTLIST_t;

typedef struct eventlist_secondary
{
	EVENTLIST_t *event;
	struct eventlist_secondary *next;
} EVENTLIST_SECONDARY_t;


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************
#define EVENT_HANDLER_TIMER_FREQUENCY_hz				(10)

#define EVENT_HANDLER_ROLLING_DISP_PERIOD_seconds		(4)

#define AUTO_ACK_PERIOD_NONE							(0x0)
#define DEFAULT_AUTO_ACK_PERIOD_ALARM_seconds			(AUTO_ACK_PERIOD_NONE)
#define DEFAULT_AUTO_ACK_PERIOD_DEVIATION_seconds		(AUTO_ACK_PERIOD_NONE)
#define DEFAULT_AUTO_ACK_PERIOD_ADVISORY_seconds		(60)

#define EVENT_HANDLER_TIMER_FLAG						(RTOS_EVENT_FLAG_1)
#define EVENT_HANDLER_PENDING_EVENT_ACK_FLAG			(RTOS_EVENT_FLAG_2)

#define EVENT_HANDLER_ALL_EVENTS						( EVENT_HANDLER_TIMER_FLAG | \
														  EVENT_HANDLER_PENDING_EVENT_ACK_FLAG )


#define NUM_EVENTLIST_ELEMENTS							(50)

#define WILDCARD_CHAR		0x2A	//*


// ***************************************************
// * PRIVATE VARIABLES
// ***************************************************
static uint8 eventHandler_TaskID;
static uint8 eventHandler_TimerID;

static uint8 eventLists_ResourceID;

static LogEventFunction_t s_logEventFunction = NULL;


static EVENTLIST_t s_eventlist[ NUM_EVENTLIST_ELEMENTS ];
static EVENTLIST_t *s_freeList;

static EVENTLIST_t *s_alarmList;
static EVENTLIST_t *s_deviationList;
static EVENTLIST_t *s_advisoryList;
static EVENTLIST_t *s_recordOnlyList;

static EVENTLIST_t *s_rollingDisplay;


static EVENTLIST_SECONDARY_t s_eventlist_Secondary[ NUM_EVENTLIST_ELEMENTS ];
static EVENTLIST_SECONDARY_t *s_freeList_Secondary;

static EVENTLIST_SECONDARY_t *s_ackList;
static EVENTLIST_SECONDARY_t *s_clearList;

static EVENTLIST_SECONDARY_t *s_eventNeedingAck;
static EVENTLIST_SECONDARY_t *s_eventAttemptingClear;


static DistVarType s_pendingEventCodeToAck = 0;


static uint16 s_rollingDisplayCounter = 0;


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************
static sint8 EventCodeCmp( DistVarType event_code_1, DistVarType event_code_2 );


static bool AddEvent( EVENTLIST_t **eventlist_ptr, DistVarType event_code, EventAckCallback_t ack_callback_function, EventClearFunction_t event_clear_function, uint8 ack_timeout_seconds );
static bool RemoveEvent( EVENTLIST_t **eventlist_ptr, DistVarType event_code );
static EVENTLIST_t* FindEvent( EVENTLIST_t **eventlist_ptr, DistVarType event_code );

static uint8 ListEvents( EVENTLIST_t **event_list_ptr, DistVarType *listBuffer);
static uint8 CountEvents( EVENTLIST_t **event_list_ptr);

static void AddEvent_Secondary( EVENTLIST_SECONDARY_t **eventlist_sec_ptr, EVENTLIST_t *event );
static void RemoveEvent_Secondary( EVENTLIST_SECONDARY_t **eventlist_sec_ptr, EVENTLIST_t *event );
static EVENTLIST_t* FindEvent_Secondary( EVENTLIST_SECONDARY_t **eventlist_sec_ptr, DistVarType event_code );


static void MaintainClearList( void );

static void UpdateEventNeedingAck( void );
static void MaintainEventNeedingAck( void );

static void UpdateRollingDisplay( void );
static void MaintainRollingDisplayList( void );


static void DumpEvents( EVENTLIST_t **event_list_ptr );
static void DumpEvents_Secondary( EVENTLIST_SECONDARY_t **event_list_ptr );




// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************
bool EVENT_HANDLER_Init( LogEventFunction_t log_event_function )
{
	uint16 i;

	s_logEventFunction = log_event_function;

	// Initialize all event lists to empty
	s_alarmList = NULL;
	s_deviationList = NULL;
	s_advisoryList = NULL;
	s_recordOnlyList = NULL;
	s_rollingDisplay = NULL;

	s_ackList = NULL;
	s_clearList = NULL;
	s_eventNeedingAck = NULL;
	s_eventAttemptingClear = NULL;

	s_freeList = NULL;
	s_freeList_Secondary = NULL;
	for( i=0; i<NUM_EVENTLIST_ELEMENTS; i++ )
	{
		s_eventlist[i].eventCode = 0;
		s_eventlist[i].ackCallback = NULL;
		s_eventlist[i].clearFunction = NULL;
		s_eventlist[i].ackTimeout_seconds = 0;
		s_eventlist[i].next = s_freeList;
		s_freeList = &s_eventlist[i];

		s_eventlist_Secondary[i].event = NULL;
		s_eventlist_Secondary[i].next = s_freeList_Secondary;
		s_freeList_Secondary = &s_eventlist_Secondary[i];
	}


	// Reserve a resource ID for the event lists
	eventLists_ResourceID = RtosResourceReserveID();		
	if( eventLists_ResourceID == RTOS_INVALID_ID )
	{
		// Failed to reserve a resource ID.  Something is seriously wrong...
		DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER_Init: RtosResourceReserveID() failed\n" );
		return FALSE;
	}

	// Reserve a task timer
	eventHandler_TimerID = RtosTimerReserveID();
	if( eventHandler_TimerID == RTOS_INVALID_ID )
	{
		// Failed to get the timer set up.  Something is seriously wrong...
		DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER_Init: RtosTimerReserveID() failed\n" );
		return FALSE;
	}

	eventHandler_TaskID = RtosTaskCreateStart( EVENT_HANDLER_Task );
	if( eventHandler_TaskID == RTOS_INVALID_ID )
	{
		// Failed to get the task fired up.  Something is seriously wrong...
		DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER_Init: RtosTaskCreateStart() failed\n" );
		return FALSE;
	}


	return TRUE;
}


bool EVENT_HANDLER_EventSet( char *event_code_str, eventtype_t event_type, 
							 EventAckCallback_t ack_callback_function,
							 EventClearFunction_t event_clear_function )
{
	uint8 autoAckPeriod = 0;
	
	switch( event_type )
	{
		case EVENTTYPE_ALARM:
			autoAckPeriod = DEFAULT_AUTO_ACK_PERIOD_ALARM_seconds;
			break;
			
		case EVENTTYPE_DEVIATION:
			autoAckPeriod = DEFAULT_AUTO_ACK_PERIOD_DEVIATION_seconds;
			break;

		case EVENTTYPE_ADVISORY:
			autoAckPeriod = DEFAULT_AUTO_ACK_PERIOD_ADVISORY_seconds;
			break;

		case EVENTTYPE_RECORD_ONLY:	
		default:
			break;
	}
	
	return EVENT_HANDLER_EventSetAutoAck( event_code_str, event_type, 
							 		ack_callback_function, event_clear_function,
							 		autoAckPeriod );
}

bool EVENT_HANDLER_EventSetAutoAck( char *event_code_str, eventtype_t event_type, 
							 		EventAckCallback_t ack_callback_function,
									EventClearFunction_t event_clear_function,
							 		uint8 auto_ack_seconds )
{
	bool log;

	DistVarType eventCode = EVENT_DEFS_EncodeEventCodeDvar( event_code_str, event_type );

	if( eventCode == 0 )
	{
		return FALSE;
	}
	
	if( auto_ack_seconds && (auto_ack_seconds != 0xFF) )
	{
		auto_ack_seconds++;
	}

	switch( event_type )
	{
		case EVENTTYPE_ALARM:
			log = AddEvent( &s_alarmList, eventCode, ack_callback_function, event_clear_function, auto_ack_seconds );
			break;

		case EVENTTYPE_DEVIATION:
			log = AddEvent( &s_deviationList, eventCode, ack_callback_function, event_clear_function, auto_ack_seconds );
			break;

		case EVENTTYPE_ADVISORY:
			log = AddEvent( &s_advisoryList, eventCode, ack_callback_function, event_clear_function, auto_ack_seconds );
			break;

		case EVENTTYPE_RECORD_ONLY:
			if( event_clear_function == NULL )
			{
				log = TRUE;
			}
			else
			{
				log = AddEvent( &s_recordOnlyList, eventCode, ack_callback_function, event_clear_function, auto_ack_seconds );
			}
			break;

		default:
			log = FALSE;
			break;
	}

	if( log && (s_logEventFunction != NULL) )
	{
		s_logEventFunction( event_code_str, event_type, EVENTACTION_SET );
	}

	return TRUE;
}

bool EVENT_HANDLER_EventAcknowledge( char *event_code_str, eventtype_t event_type )
{
	EVENTLIST_t *ackedEvent;

	DistVarType eventCode = EVENT_DEFS_EncodeEventCodeDvar( event_code_str, event_type );

	if( eventCode == 0 )
	{
		return FALSE;
	}

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		ackedEvent = FindEvent_Secondary( &s_ackList, eventCode );
		if( ackedEvent != NULL )
		{
			RemoveEvent_Secondary( &s_ackList, ackedEvent );

			if( ackedEvent->clearFunction != NULL )
			{
				AddEvent_Secondary( &s_clearList, ackedEvent );
			}

			UpdateEventNeedingAck();
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	if( ackedEvent != NULL )
	{
		if( s_logEventFunction != NULL )
		{
			s_logEventFunction( event_code_str, event_type, EVENTACTION_ACKNOWLEDGE );
		}

		if( ackedEvent->ackCallback != NULL )
		{
			ackedEvent->ackCallback( event_code_str, event_type );
		}
	}

	return TRUE;
}

DistVarType EVENT_HANDLER_EventAcknowledge_DVCB( DistVarType oldVal, DistVarType newVal )
{
	if( newVal == oldVal )
	{
		s_pendingEventCodeToAck = newVal;
		
		if( K_Event_Signal( RTOS_NOTIFY_SPECIFIC, eventHandler_TaskID, EVENT_HANDLER_PENDING_EVENT_ACK_FLAG ) != K_OK )
		{
			// This should never occur unless something got corrupted. Not much we can do about it...
			DEBUG_PRINT_STRING( DBUG_ALWAYS, "\n***EVENT_HANDLER.TriggerEventCodeAcknowledge: K_Event_Signal() failed\n" );
			// TODO: What should we do if this actually happens?
		}

		newVal = 0;
	}
	else
	{
		newVal = oldVal;
	}

	return newVal;
}

bool EVENT_HANDLER_EventClear( char *event_code_str, eventtype_t event_type )
{
	bool log;

	DistVarType eventCode = EVENT_DEFS_EncodeEventCodeDvar( event_code_str, event_type );

	if( eventCode == 0 )
	{
		return FALSE;
	}

	switch( event_type )
	{
		case EVENTTYPE_ALARM:
			log = RemoveEvent( &s_alarmList, eventCode );
			break;

		case EVENTTYPE_DEVIATION:
			log = RemoveEvent( &s_deviationList, eventCode );
			break;

		case EVENTTYPE_ADVISORY:
			log = RemoveEvent( &s_advisoryList, eventCode );
			break;

		case EVENTTYPE_RECORD_ONLY:
			(void) RemoveEvent( &s_recordOnlyList, eventCode );
			log = FALSE;
			break;

		default:
			log = FALSE;
			break;
	}

	if( log && (s_logEventFunction != NULL) )
	{
		s_logEventFunction( event_code_str, event_type, EVENTACTION_CLEAR );
	}

	return TRUE;
}


bool EVENT_HANDLER_IsEventActive( char *event_code_str, eventtype_t event_type )
{
	DistVarType eventCode = EVENT_DEFS_EncodeEventCodeDvar( event_code_str, event_type );

	EVENTLIST_t **eventListPtr;

	switch( event_type )
	{
		case EVENTTYPE_ALARM:
			eventListPtr = &s_alarmList;
			break;

		case EVENTTYPE_DEVIATION:
			eventListPtr = &s_deviationList;
			break;

		case EVENTTYPE_ADVISORY:
			eventListPtr = &s_advisoryList;
			break;
		case EVENTTYPE_RECORD_ONLY:
			eventListPtr = &s_recordOnlyList;
			break;
		default:
			return FALSE;
	}

	if( event_code_str == NULL )
	{
		return (*eventListPtr != NULL);
	}
	else
	{
		return (FindEvent(eventListPtr, eventCode) != NULL);
	}
}


DistVarType EVENT_HANDLER_GetEventCodeNeedingAck( void )
{
	DistVarType eventCode = 0;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		if( s_eventNeedingAck != NULL )
		{
			eventCode = s_eventNeedingAck->event->eventCode;
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return eventCode;
}

DistVarType EVENT_HANDLER_GetEventCodeRollingDisplay( void )
{
	DistVarType eventCode = 0;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		if( s_rollingDisplay != NULL )
		{
			eventCode = s_rollingDisplay->eventCode;
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return eventCode;
}


void EVENT_HANDLER_GCAP_DumpEventLists( void )
{
	EVENTLIST_t *eventlistScan;
	EVENTLIST_SECONDARY_t *eventlistScan_Secondary;
	uint16 freeCounter;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		freeCounter = 0;
		eventlistScan = s_freeList;
		while( eventlistScan != NULL )
		{
			freeCounter++;
			eventlistScan = eventlistScan->next;
		}
		(void)printf( "\nFREE COUNT: %d\n", freeCounter );

		freeCounter = 0;
		eventlistScan_Secondary = s_freeList_Secondary;
		while( eventlistScan_Secondary != NULL )
		{
			freeCounter++;
			eventlistScan_Secondary = eventlistScan_Secondary->next;
		}
		(void)printf( "\nFREE COUNT SECONDARY: %d\n", freeCounter );

		(void)printf( "\nALARMS\n" );
		(void)printf( "---------------\n" );
		DumpEvents( &s_alarmList );

		(void)printf( "\nDEVIATIONS\n" );
		(void)printf( "---------------\n" );
		DumpEvents( &s_deviationList );

		(void)printf( "\nADVISORIES\n" );
		(void)printf( "---------------\n" );
		DumpEvents( &s_advisoryList );

		(void)printf( "\nRECORD ONLY\n" );
		(void)printf( "---------------\n" );
		DumpEvents( &s_recordOnlyList );

		(void)printf( "\nACKNOWLEDGE\n" );
		(void)printf( "---------------\n" );
		DumpEvents_Secondary( &s_ackList );

		(void)printf( "\nCLEAR\n" );
		(void)printf( "---------------\n" );
		DumpEvents_Secondary( &s_clearList );

		(void)printf( "\n" );
	}
	(void)K_Resource_Release( eventLists_ResourceID );
}

bool EVENT_HANDLER_isInputEventListOnlyEventsActive(const char **checkForTheseEvents, uint8 sizeOfEventList, eventtype_t eventType)
{
	bool matchFound = FALSE;
	uint8 totalCount = 0;
	uint8 eventsToCheckLoop = 0;
	uint8 activeEventsLoop = 0;
	EVENTLIST_t **eventListPtr;

	char activeEventBuffer[5];
	eventtype_t activeEventTypeBuffer;

	const char **localBufferCheckForTheseEvents;

	//Make static so buffer isn't added to RTOS Stack
	static DistVarType eventCodeBuffer[NUM_EVENTLIST_ELEMENTS];

	//Decide which list of events to check
	switch( eventType )
	{
		case EVENTTYPE_ALARM:
			eventListPtr = &s_alarmList;
			break;
		case EVENTTYPE_DEVIATION:
			eventListPtr = &s_deviationList;
			break;
		case EVENTTYPE_ADVISORY:
			eventListPtr = &s_advisoryList;
			break;
		case EVENTTYPE_RECORD_ONLY:
			eventListPtr = &s_recordOnlyList;
			break;
		default:
			return FALSE;
	}

	//Get the list of active events of the previously specified type
	totalCount = ListEvents( eventListPtr, eventCodeBuffer);

	//Loop through the full list of active alarms
	for(activeEventsLoop = 0; activeEventsLoop < totalCount; activeEventsLoop++)
	{
		//Decode the Active Event Element
		EVENT_DEFS_DecodeEventCodeDvar( eventCodeBuffer[activeEventsLoop], activeEventBuffer, &activeEventTypeBuffer );

		//Check if the event is in the input list
		matchFound = FALSE;
		eventsToCheckLoop = 0;
		localBufferCheckForTheseEvents = checkForTheseEvents;
		
		while(!matchFound && (eventsToCheckLoop < sizeOfEventList))
		{
			if(strcmp(*localBufferCheckForTheseEvents, activeEventBuffer) == 0)
			{
				matchFound = TRUE;
			}

			eventsToCheckLoop++;
			localBufferCheckForTheseEvents++;
		}

		//Check if a match was found
		if(!matchFound)
		{
			//The event is not in the passed-in list.  Return FALSE!
			return FALSE;
		}
	}

	return TRUE;
}

uint8 EVENT_HANDLER_GetActiveEventCount( eventtype_t event_type )
{
	EVENTLIST_t **eventListPtr;

	//Decide which list of events to check
	switch( event_type )
	{
		case EVENTTYPE_ALARM:
			eventListPtr = &s_alarmList;
			break;
		case EVENTTYPE_DEVIATION:
			eventListPtr = &s_deviationList;
			break;
		case EVENTTYPE_ADVISORY:
			eventListPtr = &s_advisoryList;
			break;
		case EVENTTYPE_RECORD_ONLY:
			eventListPtr = &s_recordOnlyList;
			break;
		default:
			return FALSE;
	}

	//Get the list of active events of the previously specified type
	return CountEvents( eventListPtr);
}

// ***************************************************
// * TASKS
// ***************************************************
void EVENT_HANDLER_Task( void )
{
	uint8 events;

	// Set up the rolling display timer event to trigger this task
	events = K_Timer_Create( eventHandler_TimerID,
							 RTOS_NOTIFY_SPECIFIC,
							 eventHandler_TaskID,
							 EVENT_HANDLER_TIMER_FLAG );

	if( events != K_OK)
	{
		// Failed to get the timer set up.  Something is seriously wrong...
		DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER_Task: K_Timer_Create() failed\n" );
	}

	// Start the periodic timer at a rate of DVSEG_RUN_TASK_FREQ
	events |= K_Timer_Start( eventHandler_TimerID,
							 RtosGetTickFreq(),
							 (RtosGetTickFreq() / EVENT_HANDLER_TIMER_FREQUENCY_hz) );

	if( events != K_OK)
	{
		// Failed to get the timer set up.  Something is seriously wrong...
		DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER_Task: K_Timer_Start() failed\n" );
	}


	for( ;; )
	{
	   	events = K_Event_Wait( EVENT_HANDLER_ALL_EVENTS, 0, RTOS_CLEAR_EVENT_FLAGS_AFTER );

		if( events & EVENT_HANDLER_TIMER_FLAG )
		{
			MaintainClearList();
			MaintainEventNeedingAck();
			MaintainRollingDisplayList();
		}

		if( events & EVENT_HANDLER_PENDING_EVENT_ACK_FLAG )
		{
			char eventCodeStr[5];
			eventtype_t eventType;

			EVENT_DEFS_DecodeEventCodeDvar( s_pendingEventCodeToAck, eventCodeStr, &eventType );
			(void)EVENT_HANDLER_EventAcknowledge( eventCodeStr, eventType );
		}
	}
}

// ***************************************************
// * PRIVATE FUNCTIONS
// ***************************************************
static sint8 EventCodeCmp( DistVarType event_code_1, DistVarType event_code_2 )
{
	char eventCodeStr_1[5];
	eventtype_t eventType_1;
	char eventCodeStr_2[5];
	eventtype_t eventType_2;

	uint8 i;

	EVENT_DEFS_DecodeEventCodeDvar( event_code_1, eventCodeStr_1, &eventType_1 );
	EVENT_DEFS_DecodeEventCodeDvar( event_code_2, eventCodeStr_2, &eventType_2 );

	if( eventType_1 < eventType_2 )
	{
		return 1;
	}
	else if( eventType_1 > eventType_2 )
	{
		return -1;
	}

	for( i=0; i<4; i++ )
	{
		if( eventCodeStr_1[i] > eventCodeStr_2[i] )
		{
			return 1;
		}
		else if( eventCodeStr_1[i] < eventCodeStr_2[i] )
		{
			return -1;
		}
	}

	return 0;
}

static bool AddEvent( EVENTLIST_t **event_list_ptr, DistVarType event_code, EventAckCallback_t ack_callback_function, EventClearFunction_t event_clear_function, uint8 ack_timeout_seconds )
{
	EVENTLIST_t *newEvent = s_freeList;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		if( *event_list_ptr == NULL )
		{
			// event list is currently empty, so just need point the list head at the new element

			if( newEvent == NULL )
			{
				// we're out of free list elements!
				DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER.AddEvent(): free pool is empty.\n" );
				(void)K_Resource_Release( eventLists_ResourceID );
				return FALSE;
			}
		
			// We're taking an event off the freeList...
			s_freeList = s_freeList->next;

			*event_list_ptr = newEvent;
			newEvent->next = NULL;
		}
		else
		{
			EVENTLIST_t *eventListScan_prev = NULL;
			EVENTLIST_t *eventListScan = *event_list_ptr;
	
			// Find the right spot to insert the new element
			while( (eventListScan != NULL) &&
				   (EventCodeCmp(event_code, eventListScan->eventCode) > 0) )
			{
				eventListScan_prev = eventListScan;
				eventListScan = eventListScan->next;
			}

			if( (eventListScan != NULL) &&
				(event_code == eventListScan->eventCode) )
			{
				// This event is already set.
				(void)K_Resource_Release( eventLists_ResourceID );
				return FALSE;
			}

			if( s_freeList == NULL )
			{
				// we're out of free list elements!
				DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER.AddEvent(): free pool is empty.\n" );
				(void)K_Resource_Release( eventLists_ResourceID );
				return FALSE;
			}
		
			// We're taking an event off the freeList...
			s_freeList = s_freeList->next;	
		
			// insert the new element
			if( eventListScan_prev == NULL )
			{
				// we are still at first element, so need to update the list head pointer
				// to point at new element
				*event_list_ptr = newEvent;
			}
			else
			{
				// we are somewhere past the first element, so update the previous element
				eventListScan_prev->next = newEvent;
			}
			newEvent->next = eventListScan;		
		}

		// Load the data into the new element
		newEvent->eventCode = event_code;
		newEvent->ackCallback = ack_callback_function;
		newEvent->clearFunction = event_clear_function;
		newEvent->ackTimeout_seconds = ack_timeout_seconds;

		if(*event_list_ptr == s_recordOnlyList)
		{
			AddEvent_Secondary( &s_clearList, newEvent );
		}
		else
		{
			AddEvent_Secondary( &s_ackList, newEvent );
		}

		UpdateEventNeedingAck();
		UpdateRollingDisplay();
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return TRUE;
}

static bool RemoveEvent( EVENTLIST_t **eventlist_ptr, DistVarType event_code )
{
	EVENTLIST_t *eventListScan_prev;
	EVENTLIST_t *eventListScan;
	EVENTLIST_t *freedElement;

	bool found = FALSE;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		if( *eventlist_ptr == NULL )
		{
			// list is already empty...
			(void)K_Resource_Release( eventLists_ResourceID );
			return FALSE;
		}
	
		// set scan pointer to event list head
		eventListScan = *eventlist_ptr;
		eventListScan_prev = NULL;
	
		while( eventListScan != NULL )
		{
			if( event_code == eventListScan->eventCode )
			{
				// Found the event_code...
				found = TRUE;
	
				// Make note of the element that needs to be removed
				// so that we can adjust the links of adjacent elements to skip over it.
				freedElement = eventListScan;
	
				// Adjust adjacent elements to skip over element to be removed...
				if( eventListScan == *eventlist_ptr )
				{
					// We're at head of the event list, so need to maintain list's head pointer by
					// pointing it at next element in list
					*eventlist_ptr = eventListScan->next;
				}
	
				// Update previous element to skip over removed element
				if( eventListScan_prev != NULL )
				{
					eventListScan_prev->next = eventListScan->next;
				}
	
				// move scan pointer on to next element
				eventListScan = eventListScan->next;
	
				// Check if s_rollingDisplay is pointing to the freed element and advance to next if needed
				if( s_rollingDisplay == freedElement )
				{
					s_rollingDisplay = eventListScan;
					s_rollingDisplayCounter = 0;
				}

				// Make sure event is removed from ack and clear lists
				RemoveEvent_Secondary( &s_ackList, freedElement );
				RemoveEvent_Secondary( &s_clearList, freedElement );
	
				// Put the freed element back on the head of the free pool
				freedElement->eventCode = 0;
				freedElement->ackCallback = NULL;
				freedElement->clearFunction = NULL;
				freedElement->ackTimeout_seconds = 0;
				freedElement->next = s_freeList;
				s_freeList = freedElement;

				UpdateEventNeedingAck();
				UpdateRollingDisplay();
			}
			else
			{
				// No match, move scan pointers on to next element
	
				eventListScan_prev = eventListScan;
				eventListScan = eventListScan->next;
			}
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return found;
}

static EVENTLIST_t* FindEvent( EVENTLIST_t **event_list_ptr, DistVarType event_code )
{
	EVENTLIST_t *eventListScan = *event_list_ptr;
	DistVarType eventCodeMask = 0x00000000;
	uint8 i;
	
	// Set up eventCodeMask to look at only non-wildcard characters.
	//Note: event_code has been bitpacked so each character is represented by 7 bits
	for ( i = 0 ; i < 28 ; i+=7)
	{
		if( (uint8)((event_code>>i)&0x7F) != WILDCARD_CHAR )
		{
			eventCodeMask |= ((DistVarType)0x7F << i );
		} 
	}
	
	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		while( eventListScan != NULL )
		{
			if( (event_code & eventCodeMask) == (eventListScan->eventCode & eventCodeMask) )
			{
				// Found it...
				break;
			}
	
			eventListScan = eventListScan->next;
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return eventListScan;
}

static uint8 ListEvents( EVENTLIST_t **event_list_ptr, DistVarType *listBuffer)
{
	EVENTLIST_t *eventListScan = *event_list_ptr;
	uint8 returnCount = 0;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		while( eventListScan != NULL )
		{
			*listBuffer = eventListScan->eventCode;

			eventListScan = eventListScan->next;
			listBuffer++;
			returnCount++;
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return returnCount;
}

static uint8 CountEvents( EVENTLIST_t **event_list_ptr)
{
	EVENTLIST_t *eventListScan = *event_list_ptr;
	uint8 returnCount = 0;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		while( eventListScan != NULL )
		{
			eventListScan = eventListScan->next;
			returnCount++;
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	return returnCount;
}

static void AddEvent_Secondary( EVENTLIST_SECONDARY_t **eventlist_sec_ptr, EVENTLIST_t *event )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	EVENTLIST_SECONDARY_t *newEvent_Secondary = s_freeList_Secondary;

	if( *eventlist_sec_ptr == NULL )
	{
		// event list is currently empty, so just need point the list head at the new element

		if( s_freeList_Secondary == NULL )
		{
			// we're out of free list elements!
			DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER.AddEvent_Secondary(): free pool is empty.\n" );
			return;
		}
	
		// We're taking an event off the freeList...
		s_freeList_Secondary = s_freeList_Secondary->next;

		*eventlist_sec_ptr = newEvent_Secondary;
		newEvent_Secondary->next = NULL;
	}
	else
	{
		EVENTLIST_SECONDARY_t *eventListSecScan_prev = NULL;
		EVENTLIST_SECONDARY_t *eventListSecScan = *eventlist_sec_ptr;

		// Find the right spot to insert the new element
		while( (eventListSecScan != NULL) &&
			   (EventCodeCmp(event->eventCode, eventListSecScan->event->eventCode) > 0) )
		{
			eventListSecScan_prev = eventListSecScan;
			eventListSecScan = eventListSecScan->next;
		}

		if( (eventListSecScan != NULL) &&
			(event->eventCode == eventListSecScan->event->eventCode) )
		{
			// This event is already set.
			return;
		}

		if( s_freeList_Secondary == NULL )
		{
			// we're out of free list elements!
			DEBUG_PRINT_STRING( DBUG_ALWAYS, "EVENT_HANDLER.AddEvent_Secondary(): free pool is empty.\n" );
			return;
		}
	
		// We're taking an event off the freeList...
		s_freeList_Secondary = s_freeList_Secondary->next;

		// insert the new element
		if( eventListSecScan_prev == NULL )
		{
			// we are still at first element, so need to update the list head pointer
			// to point at new element
			*eventlist_sec_ptr = newEvent_Secondary;
		}
		else
		{
			// we are somewhere past the first element, so update the previous element
			eventListSecScan_prev->next = newEvent_Secondary;
		}
		newEvent_Secondary->next = eventListSecScan;		
	}

	// Load the data into the new element
	newEvent_Secondary->event = event;
}
static void RemoveEvent_Secondary( EVENTLIST_SECONDARY_t **eventlist_sec_ptr, EVENTLIST_t *event )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	EVENTLIST_SECONDARY_t *eventlistSecScan_prev;
	EVENTLIST_SECONDARY_t *eventlistSecScan;
	EVENTLIST_SECONDARY_t *freedElement;

	if( *eventlist_sec_ptr == NULL )
	{
		// list is already empty...
		return;
	}

	// set scan pointer to event list head
	eventlistSecScan = *eventlist_sec_ptr;
	eventlistSecScan_prev = NULL;

	while( eventlistSecScan != NULL )
	{
		if( event == eventlistSecScan->event )
		{
			// Found the event in the list.  Now to get rid of it...

			// Make note of the element that needs to be removed
			// so that we can adjust the links of adjacent elements to skip over it.
			freedElement = eventlistSecScan;

			// Adjust adjacent elements to skip over element to be removed...
			if( eventlistSecScan == *eventlist_sec_ptr )
			{
				// We're at head of the event list, so need to maintain list's head pointer by
				// pointing it at next element in list
				*eventlist_sec_ptr = eventlistSecScan->next;
			}

			// Update previous element to skip over removed element
			if( eventlistSecScan_prev != NULL )
			{
				eventlistSecScan_prev->next = eventlistSecScan->next;
			}

			// move scan pointer on to next element
			eventlistSecScan = eventlistSecScan->next;

			// Check if s_eventAttemptingClear is pointing to the freed element and advance to next if needed
			if( (s_eventAttemptingClear != NULL) &&
				(s_eventAttemptingClear->event == freedElement->event) )
			{
				s_eventAttemptingClear = s_eventAttemptingClear->next;
			}

			// Put the freed element back on the head of the free pool
			freedElement->event = NULL;
			freedElement->next = s_freeList_Secondary;
			s_freeList_Secondary = freedElement;
		}
		else
		{
			// No match, move scan pointers on to next element

			eventlistSecScan_prev = eventlistSecScan;
			eventlistSecScan = eventlistSecScan->next;
		}
	}
}

static EVENTLIST_t* FindEvent_Secondary( EVENTLIST_SECONDARY_t **eventlist_sec_ptr, DistVarType event_code )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	EVENTLIST_SECONDARY_t *eventListSecScan = *eventlist_sec_ptr;

	while( eventListSecScan != NULL )
	{
		if( event_code == eventListSecScan->event->eventCode )
		{
			// Found it...
			return eventListSecScan->event;
		}

		eventListSecScan = eventListSecScan->next;
	}

	return NULL;
}


static void MaintainClearList( void )
{
	static uint8 iterationCounter = 0;

	uint8 clearListCount;
	uint8 clearAttempts;

	EVENTLIST_SECONDARY_t *eventListSecScan;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		clearListCount = 0;
		eventListSecScan = s_clearList;
		while( eventListSecScan != NULL )
		{
			clearListCount++;
			eventListSecScan = eventListSecScan->next;
		}

		if( clearListCount == 0 )
		{
			iterationCounter = 0;
			(void)K_Resource_Release( eventLists_ResourceID );
			return;
		}

		clearListCount--;
		clearAttempts = clearListCount / EVENT_HANDLER_TIMER_FREQUENCY_hz;
		if( (clearListCount % EVENT_HANDLER_TIMER_FREQUENCY_hz) >= iterationCounter )
		{
			clearAttempts++;
		}

		iterationCounter++;
		if( iterationCounter >= EVENT_HANDLER_TIMER_FREQUENCY_hz )
		{
			iterationCounter = 0;
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );

	for( ; clearAttempts > 0; clearAttempts-- )
	{
		char eventCodeStr[5];
		eventtype_t eventType = EVENTTYPE_INVALID;
		EventClearFunction_t clearFunction = NULL;

		(void)K_Resource_Wait( eventLists_ResourceID, 0 );
		{
			if( s_eventAttemptingClear == NULL )
			{
				s_eventAttemptingClear = s_clearList;
			}

			if( (s_eventAttemptingClear != NULL) &&
				(s_eventAttemptingClear->event != NULL) )
			{
				clearFunction = s_eventAttemptingClear->event->clearFunction;
				EVENT_DEFS_DecodeEventCodeDvar( s_eventAttemptingClear->event->eventCode, eventCodeStr, &eventType );

				s_eventAttemptingClear = s_eventAttemptingClear->next;
			}
		}
		(void)K_Resource_Release( eventLists_ResourceID );

		if( (clearFunction != NULL) &&
			clearFunction(eventCodeStr, eventType) )
		{
			(void)EVENT_HANDLER_EventClear( eventCodeStr, eventType );
		}
	}
}


static void UpdateEventNeedingAck( void )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	if( (s_ackList == NULL) ||
		(s_eventNeedingAck == NULL) ||
		(s_eventNeedingAck->event == NULL) ||
		(FindEvent_Secondary(&s_ackList, s_eventNeedingAck->event->eventCode) == NULL) )
	{
		s_eventNeedingAck = s_ackList;
	}
	else
	{
		char eventCodeStr_1[5];
		eventtype_t eventType_1;
		char eventCodeStr_2[5];
		eventtype_t eventType_2;
		
		EVENT_DEFS_DecodeEventCodeDvar( s_ackList->event->eventCode, eventCodeStr_1, &eventType_1 );
		EVENT_DEFS_DecodeEventCodeDvar( s_eventNeedingAck->event->eventCode, eventCodeStr_2, &eventType_2 );

		if( eventType_1 > eventType_2 )
		{
			s_eventNeedingAck = s_ackList;
		}
	}
}

static void MaintainEventNeedingAck( void )
{
	static uint8 oneSecondCounter = 0;
	eventtype_t eventType;
	EVENTLIST_t **eventListPtr;
	EVENTLIST_t *eventlistScan;
	
	if( ++oneSecondCounter < EVENT_HANDLER_TIMER_FREQUENCY_hz )
	{
		return;
	}
	oneSecondCounter = 0;
	
	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		for( eventType = (eventtype_t)0; eventType < EVENTTYPE_NUM_TYPES; eventType++ )
		{
			switch( eventType )
			{
				case EVENTTYPE_ALARM:
					eventListPtr = &s_alarmList;
					break;
		
				case EVENTTYPE_DEVIATION:
					eventListPtr = &s_deviationList;
					break;
		
				case EVENTTYPE_ADVISORY:
					eventListPtr = &s_advisoryList;
					break;
		
				default:
					eventListPtr = NULL;
					break;
			}
			
			if( eventListPtr != NULL )
			{
				eventlistScan = *eventListPtr;
				
				while( eventlistScan != NULL )
				{
					if( eventlistScan->ackTimeout_seconds != AUTO_ACK_PERIOD_NONE )
					{
						eventlistScan->ackTimeout_seconds--;
						
						if( !eventlistScan->ackTimeout_seconds )
						{
							char eventCodeStr[5];
							eventtype_t tempEventType;
							
							EVENT_DEFS_DecodeEventCodeDvar( eventlistScan->eventCode, eventCodeStr, &tempEventType );
	
							(void)EVENT_HANDLER_EventAcknowledge_DVCB( eventlistScan->eventCode, eventlistScan->eventCode );
						}
					}
					
					eventlistScan = eventlistScan->next;
				}
			}
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );
}


static void UpdateRollingDisplay( void )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	bool reload = FALSE;

	if( (s_rollingDisplay == NULL) ||
		(s_rollingDisplay->eventCode == 0) )
	{
		reload = TRUE;
	}
	else
	{
		char eventCodeStr[5];
		eventtype_t eventType;
		
		EVENT_DEFS_DecodeEventCodeDvar( s_rollingDisplay->eventCode, eventCodeStr, &eventType );

		if( ((s_alarmList != NULL) && (eventType < EVENTTYPE_ALARM)) ||
 			((s_deviationList != NULL) && (eventType < EVENTTYPE_DEVIATION)) ||
 			((s_advisoryList != NULL) && (eventType < EVENTTYPE_ADVISORY)) )
		{
			reload = TRUE;
		}
	}

	if( reload )
	{
		if( s_alarmList != NULL )
		{
			s_rollingDisplay = s_alarmList;
		}
		else if( s_deviationList != NULL )
		{
			s_rollingDisplay = s_deviationList;
		}
		else if( s_advisoryList != NULL )
		{
			s_rollingDisplay = s_advisoryList;
		}
		else
		{
			s_rollingDisplay = NULL;
		}

		s_rollingDisplayCounter = 0;
	}
}


static void MaintainRollingDisplayList( void )
{
	s_rollingDisplayCounter++;
	if( s_rollingDisplayCounter <= (EVENT_HANDLER_ROLLING_DISP_PERIOD_seconds * EVENT_HANDLER_TIMER_FREQUENCY_hz) )
	{
		return;
	}
	s_rollingDisplayCounter = 0;

	(void)K_Resource_Wait( eventLists_ResourceID, 0 );
	{
		if( (s_rollingDisplay != NULL) &&
			(s_rollingDisplay->next != NULL) )
		{
			s_rollingDisplay = s_rollingDisplay->next;
		}
		else
		{
			if( s_alarmList != NULL )
			{
				s_rollingDisplay = s_alarmList;
			}
			else if( s_deviationList != NULL )
			{
				s_rollingDisplay = s_deviationList;
			}
			else if( s_advisoryList != NULL )
			{
				s_rollingDisplay = s_advisoryList;
			}
			else
			{
				s_rollingDisplay = NULL;
			}
		}
	}
	(void)K_Resource_Release( eventLists_ResourceID );
}


static void DumpEvents( EVENTLIST_t **event_list_ptr )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	EVENTLIST_t *eventlistScan = *event_list_ptr;
	char eventCodeStr[5];
	eventtype_t eventType;
	char eventTypeChar;

	while( eventlistScan != NULL )
	{
		EVENT_DEFS_DecodeEventCodeDvar( eventlistScan->eventCode, eventCodeStr, &eventType );

		switch( eventType )
		{
			case EVENTTYPE_RECORD_ONLY:
				eventTypeChar = 'R';
				break;
	
			case EVENTTYPE_ADVISORY:
				eventTypeChar = 'V';
				break;
				
			case EVENTTYPE_DEVIATION:
				eventTypeChar = 'D';
				break;
				
			case EVENTTYPE_ALARM:
				eventTypeChar = 'A';
				break;
				
			default:
				eventTypeChar = '?';
				break;
		}	

		(void)printf( "%s-%c\n", eventCodeStr, eventTypeChar );

		eventlistScan = eventlistScan->next;
	}
}

static void DumpEvents_Secondary( EVENTLIST_SECONDARY_t **event_list_ptr )
{
	// ******************************************************************************
	// *** THE EVENT LIST RESOURCE *MUST* BE RESERVED BEFORE CALLING THIS FUNCTION
	// ******************************************************************************

	EVENTLIST_SECONDARY_t *eventlistSecScan = *event_list_ptr;
	char eventCodeStr[5];
	eventtype_t eventType;
	char eventTypeChar;

	while( eventlistSecScan != NULL )
	{
		EVENT_DEFS_DecodeEventCodeDvar( eventlistSecScan->event->eventCode, eventCodeStr, &eventType );

		switch( eventType )
		{
			case EVENTTYPE_RECORD_ONLY:
				eventTypeChar = 'R';
				break;
	
			case EVENTTYPE_ADVISORY:
				eventTypeChar = 'V';
				break;
				
			case EVENTTYPE_DEVIATION:
				eventTypeChar = 'D';
				break;
				
			case EVENTTYPE_ALARM:
				eventTypeChar = 'A';
				break;
				
			default:
				eventTypeChar = '?';
				break;
		}	

		(void)printf( "%s-%c\n", eventCodeStr, eventTypeChar );

		eventlistSecScan = eventlistSecScan->next;
	}
}
