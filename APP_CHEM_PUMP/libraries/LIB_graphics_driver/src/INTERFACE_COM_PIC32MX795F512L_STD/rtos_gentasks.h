// rtos_gentasks.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION

#include "cpstruct.h"           // needed for ROM_TCB structure definition

#ifndef GCA_APP_NO_DVAR
	#include "dvar.h"
#endif

// *****************************************************************************
// Declare variables for RTOS and DVar libraries based on application settings and/or default values
// *****************************************************************************

// Application can define the following in main.c (prior to rtos_gentasks.h being
// included) to override the default values.
// NOTE: Current buffer utilization can be viewed by typing "bufstats" into GCA portal.

//#define GCA_APP_INTERRUPT_TASK_SIZE		1024/* Size of interrupt task */
//#define GCA_APP_RTOS_MAX_RESOURCES		40	/* Maximum number of resources. */
//#define GCA_APP_RTOS_MAX_CYCLIC_TIMERS	40	/* Maximum number of cyclic timers. */
//#define GCA_APP_RTOS_MAX_MESSAGES			200	/* Maximum number of messages. */
//#define GCA_APP_RTOS_MAX_MAILBOXES		40	/* Maximum number of mailboxes. */
//#define GCA_APP_RTOS_MAX_SEMAPHORES		40	/* Maximum number of semaphores. */
//#define GCA_APP_DVAR_TRACKING_SEGMENTS	64	/* Maximum number of dvar segments that are registerable */
//#define GCA_APP_DVAR_OWNING_SEGMENTS		64	/* Maximum number of dvar segments that are ownable */

// Default variable sizes
#define DEFAULT_GCA_APP_INTERRUPT_TASK_SIZE		1024	/* Size of interrupt task */
#define DEFAULT_GCA_APP_RTOS_MAX_RESOURCES		40	/* Maximum number of resources. */
#define DEFAULT_GCA_APP_RTOS_MAX_CYCLIC_TIMERS	40	/* Maximum number of cyclic timers. */
#define DEFAULT_GCA_APP_RTOS_MAX_MESSAGES		200	/* Maximum number of messages. */
#define DEFAULT_GCA_APP_RTOS_MAX_MAILBOXES		40	/* Maximum number of mailboxes. */
#define DEFAULT_GCA_APP_RTOS_MAX_SEMAPHORES		40	/* Maximum number of semaphores. */
#define DEFAULT_GCA_APP_DVAR_TRACKING_SEGMENTS	64	/* Maximum number of dvar segments that are registerable */
#define DEFAULT_GCA_APP_DVAR_OWNING_SEGMENTS	64	/* Maximum number of dvar segments that are ownable */

// *****************************************************************************
// INTERRUPT TASK STACK SIZE
// *****************************************************************************

#ifdef GCA_APP_INTERRUPT_TASK_SIZE
	#if (GCA_APP_INTERRUPT_TASK_SIZE < DEFAULT_GCA_APP_INTERRUPT_TASK_SIZE)
		#warning GCA_APP_INTERRUPT_TASK_SIZE is less than the recommened value!
	#endif
#else
	#define GCA_APP_INTERRUPT_TASK_SIZE	(DEFAULT_GCA_APP_INTERRUPT_TASK_SIZE)
#endif

word32 interrupt_bytes[GCA_APP_INTERRUPT_TASK_SIZE / 4] __attribute__((aligned(8)));
word16 g_interrupt_bytes_sizeof = sizeof(interrupt_bytes);

// *****************************************************************************
// TASK TRACKING STRUCTURES
// *****************************************************************************

#ifndef NUM_TASKS
    #error NUM_TASKS is not defined.
#endif

#if ((NUM_TASKS < 0) || (NUM_TASKS > 31))
    #error NUM_TASKS must be between 0 and 31.
#endif

struct _tcb cmx_tcb[NUM_TASKS+1] __attribute__((aligned(4)));
uint16 gTaskStackSize[NUM_TASKS+1] = {(GCA_APP_INTERRUPT_TASK_SIZE/4), 0};
byte MAX_TASKS __attribute__((address(0xA001FF58))) = NUM_TASKS+1;
byte ROM_TASKS __attribute__((address(0xA001FF5C))) = NUM_TASKS+1;
word16 g_cmx_tcb_sizeof = sizeof(cmx_tcb);

// *****************************************************************************
// RTOS RESOURCES
// *****************************************************************************

#ifdef GCA_APP_RTOS_MAX_RESOURCES
	#if (GCA_APP_RTOS_MAX_RESOURCES > DEFAULT_GCA_APP_RTOS_MAX_RESOURCES)
		#warning GCA_APP_RTOS_MAX_RESOURCES exceeds recommened value!
	#elif ( GCA_APP_RTOS_MAX_RESOURCES >= 0xFF )
		#error GCA_APP_RTOS_MAX_RESOURCES exceeds maximum value!
	#endif
#else
	#define GCA_APP_RTOS_MAX_RESOURCES	(DEFAULT_GCA_APP_RTOS_MAX_RESOURCES)
#endif

byte MAX_RESOURCES = GCA_APP_RTOS_MAX_RESOURCES;
byte g_NUM_MAX_RESOURCES = GCA_APP_RTOS_MAX_RESOURCES;
#if GCA_APP_RTOS_MAX_RESOURCES > 0
	RESHDR res_que[GCA_APP_RTOS_MAX_RESOURCES] __attribute__((aligned(4)));
#else
	byte res_que[1];
#endif
word16 g_res_que_sizeof = sizeof(res_que);

// *****************************************************************************
// RTOS CYCLIC TIMERS
// *****************************************************************************

#ifdef GCA_APP_RTOS_MAX_CYCLIC_TIMERS
	#if (GCA_APP_RTOS_MAX_CYCLIC_TIMERS > DEFAULT_GCA_APP_RTOS_MAX_CYCLIC_TIMERS)
		#warning GCA_APP_RTOS_MAX_CYCLIC_TIMERS exceeds recommened value!
	#elif ( GCA_APP_RTOS_MAX_CYCLIC_TIMERS >= 0xFF )
		#error GCA_APP_RTOS_MAX_CYCLIC_TIMERS exceeds maximum value!
	#endif
#else
	#define GCA_APP_RTOS_MAX_CYCLIC_TIMERS	(DEFAULT_GCA_APP_RTOS_MAX_CYCLIC_TIMERS)
#endif

byte MAX_CYCLIC_TIMERS = GCA_APP_RTOS_MAX_CYCLIC_TIMERS;
#if GCA_APP_RTOS_MAX_RESOURCES > 0
	CYCLIC_TIMERS tcproc[GCA_APP_RTOS_MAX_CYCLIC_TIMERS] __attribute__((aligned(4)));
#else
	byte tcproc[1];
#endif
word16 g_tcproc_sizeof = sizeof(tcproc);

// *****************************************************************************
// RTOS MESSAGES
// *****************************************************************************

#ifdef GCA_APP_RTOS_MAX_MESSAGES
	#if (GCA_APP_RTOS_MAX_MESSAGES > DEFAULT_GCA_APP_RTOS_MAX_MESSAGES)
		#warning GCA_APP_RTOS_MAX_MESSAGES exceeds recommened value!
	#elif ( GCA_APP_RTOS_MAX_MESSAGES >= 0xFFFF )
		#error GCA_APP_RTOS_MAX_MESSAGES exceeds maximum value!
	#endif
#else
	#define GCA_APP_RTOS_MAX_MESSAGES	(DEFAULT_GCA_APP_RTOS_MAX_MESSAGES)
#endif

#if GCA_APP_RTOS_MAX_MESSAGES > 0
	#define GCA_APP_ACTUAL_RTOS_MAX_MESSAGES (GCA_APP_RTOS_MAX_MESSAGES + 2)
#else
	#define GCA_APP_ACTUAL_RTOS_MAX_MESSAGES 0
#endif

#if GCA_APP_ACTUAL_RTOS_MAX_MESSAGES > 65535
	#undef GCA_APP_ACTUAL_RTOS_MAX_MESSAGES
	#define GCA_APP_ACTUAL_RTOS_MAX_MESSAGES 65535
#endif

#if GCA_APP_ACTUAL_RTOS_MAX_MESSAGES > 0
	MSG message_box[GCA_APP_ACTUAL_RTOS_MAX_MESSAGES] __attribute__((aligned(4)));
#else
	byte message_box[1];
#endif
word16 gGcaAppActualRtosMaxMessages = GCA_APP_ACTUAL_RTOS_MAX_MESSAGES;
word16 g_message_box_sizeof = sizeof(message_box);

// *****************************************************************************
// RTOS MAILBOXES
// *****************************************************************************

#ifdef GCA_APP_RTOS_MAX_MAILBOXES
	#if (GCA_APP_RTOS_MAX_MAILBOXES > DEFAULT_GCA_APP_RTOS_MAX_MAILBOXES)
		#warning GCA_APP_RTOS_MAX_MAILBOXES exceeds recommened value!
	#elif ( GCA_APP_RTOS_MAX_MAILBOXES >= 0xFF )
		#error GCA_APP_RTOS_MAX_MAILBOXES exceeds maximum value!
	#endif
#else
	#define GCA_APP_RTOS_MAX_MAILBOXES	(DEFAULT_GCA_APP_RTOS_MAX_MAILBOXES)
#endif

byte MAX_MAILBOXES = GCA_APP_RTOS_MAX_MAILBOXES;
#if GCA_APP_RTOS_MAX_MAILBOXES > 0
	MAILBOX mail_box[GCA_APP_RTOS_MAX_MAILBOXES] __attribute__((aligned(4)));
#else
	byte mail_box[1];
#endif
word16 g_mail_box_sizeof = sizeof(mail_box);

// *****************************************************************************
// RTOS SEMAPHORES
// *****************************************************************************

#ifdef GCA_APP_RTOS_MAX_SEMAPHORES
	#if (GCA_APP_RTOS_MAX_SEMAPHORES > DEFAULT_GCA_APP_RTOS_MAX_SEMAPHORES)
		#warning GCA_APP_RTOS_MAX_SEMAPHORES exceeds recommened value!
	#elif ( GCA_APP_RTOS_MAX_SEMAPHORES >= 0xFF )
		#error GCA_APP_RTOS_MAX_SEMAPHORES exceeds maximum value!
	#endif
#else
	#define GCA_APP_RTOS_MAX_SEMAPHORES	(DEFAULT_GCA_APP_RTOS_MAX_SEMAPHORES)
#endif

byte MAX_SEMAPHORES = GCA_APP_RTOS_MAX_SEMAPHORES;
#if GCA_APP_RTOS_MAX_SEMAPHORES > 0
	SEM sem_array[GCA_APP_RTOS_MAX_SEMAPHORES] __attribute__((aligned(4)));
#else
	byte sem_array[1];
#endif
word16 g_sem_array_sizeof = sizeof(sem_array);

// *****************************************************************************
// DVAR TRACKING SEGMENTS
// *****************************************************************************
#ifndef GCA_APP_NO_DVAR

	#ifdef GCA_APP_DVAR_TRACKING_SEGMENTS
		#if (GCA_APP_DVAR_TRACKING_SEGMENTS > DEFAULT_GCA_APP_DVAR_TRACKING_SEGMENTS)
			#warning GCA_APP_DVAR_TRACKING_SEGMENTS exceeds recommened value!
		#elif ( GCA_APP_DVAR_TRACKING_SEGMENTS >= 0xFF )
			#error GCA_APP_RTOS_MAX_SEMAPHORES exceeds maximum value!
		#endif
	#else
		#define GCA_APP_DVAR_TRACKING_SEGMENTS	(DEFAULT_GCA_APP_DVAR_TRACKING_SEGMENTS)
	#endif

	#if GCA_APP_DVAR_TRACKING_SEGMENTS > 0
		VarTrackingItem g_VarTrackingList[GCA_APP_DVAR_TRACKING_SEGMENTS] __attribute__((aligned(4)));
	#else
		VarTrackingItem g_VarTrackingList[1];
	#endif
	word16 g_NumVarTrackingItems = GCA_APP_DVAR_TRACKING_SEGMENTS;
#endif

// *****************************************************************************
// DVAR OWNING SEGMENTS
// *****************************************************************************
#ifndef GCA_APP_NO_DVAR
	#ifdef GCA_APP_DVAR_OWNING_SEGMENTS
		#if (GCA_APP_DVAR_OWNING_SEGMENTS > DEFAULT_GCA_APP_DVAR_OWNING_SEGMENTS)
			#warning GCA_APP_DVAR_OWNING_SEGMENTS exceeds recommened value!
		#elif ( GCA_APP_DVAR_OWNING_SEGMENTS >= 0xFF )
			#error GCA_APP_DVAR_OWNING_SEGMENTS exceeds maximum value!
		#endif
	#else
		#define GCA_APP_DVAR_OWNING_SEGMENTS	(DEFAULT_GCA_APP_DVAR_OWNING_SEGMENTS)
	#endif

	#if GCA_APP_DVAR_OWNING_SEGMENTS > 0
		OwnerTrackingItem g_OwnerTrackingList[GCA_APP_DVAR_OWNING_SEGMENTS] __attribute__((aligned(4)));
	#else
		OwnerTrackingItem g_OwnerTrackingList[1];
	#endif
	word16 g_NumOwnerTrackingItems = GCA_APP_DVAR_OWNING_SEGMENTS;
#endif

// *****************************************************************************

#ifndef NUM_TASKS
    #error NUM_TASKS is not defined.
#endif

#if ((NUM_TASKS < 0) || (NUM_TASKS > 31))
    #error NUM_TASKS must be between 0 and 31.
#endif

#define TASK_GEN_PROTOTYPE(functionname) extern void functionname(void)

#define TASK_STACK_SIZE_MIN    50
#define TASK_PRIORITY_MIN_NUM  1
#define TASK_PRIORITY_MAX_NUM  254

#define TASK_0_FUNCTION_NAME    K_I_Timer_Task
#define TASK_0_STACK_START      0 
#define TASK_0_STACK_SIZE       0
#define TASK_0_PRIORITY         0
TASK_GEN_PROTOTYPE(TASK_0_FUNCTION_NAME);

#if (NUM_TASKS >= 1)
    #ifndef TASK_1_FUNCTION_NAME
        #error TASK_1_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_1_STACK_SIZE
        #error TASK_1_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_1_PRIORITY
        #error TASK_1_PRIORITY is not defined.
    #endif
    #if ((TASK_1_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_1_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_1_PRIORITY is out of range.
    #endif
    #if ((TASK_1_PRIORITY == TASK_0_PRIORITY))
        #error TASK_1_PRIORITY level has previously been assigned.
    #endif
    #define TASK_1_STACK_START    TASK_0_STACK_START + TASK_0_STACK_SIZE  //task 0 does not need a stack
    TASK_GEN_PROTOTYPE(TASK_1_FUNCTION_NAME);
#elif defined(TASK_1_FUNCTION_NAME) || defined(TASK_1_STACK_SIZE) || defined(TASK_1_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_1_STACK_START    TASK_0_STACK_START + TASK_0_STACK_SIZE
    #define TASK_1_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 2)
    #ifndef TASK_2_FUNCTION_NAME
        #error TASK_2_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_2_STACK_SIZE
        #error TASK_2_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_2_PRIORITY
        #error TASK_2_PRIORITY is not defined.
    #endif
    #if ((TASK_2_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_2_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_2_PRIORITY is out of range.
    #endif
    #if ((TASK_2_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_2_PRIORITY == TASK_1_PRIORITY))
        #error TASK_2_PRIORITY level has previously been assigned.
    #endif
    #define TASK_2_STACK_START    TASK_1_STACK_START + TASK_1_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_2_FUNCTION_NAME);
#elif defined(TASK_2_FUNCTION_NAME) || defined(TASK_2_STACK_SIZE) || defined(TASK_2_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_2_STACK_START    TASK_1_STACK_START + TASK_1_STACK_SIZE
    #define TASK_2_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 3)
    #ifndef TASK_3_FUNCTION_NAME
        #error TASK_3_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_3_STACK_SIZE
        #error TASK_3_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_3_PRIORITY
        #error TASK_3_PRIORITY is not defined.
    #endif
    #if ((TASK_3_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_3_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_3_PRIORITY is out of range.
    #endif
    #if ((TASK_3_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_3_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_3_PRIORITY == TASK_2_PRIORITY))
        #error TASK_3_PRIORITY level has previously been assigned.
    #endif
    #define TASK_3_STACK_START    TASK_2_STACK_START + TASK_2_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_3_FUNCTION_NAME);
#elif defined(TASK_3_FUNCTION_NAME) || defined(TASK_3_STACK_SIZE) || defined(TASK_3_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_3_STACK_START    TASK_2_STACK_START + TASK_2_STACK_SIZE
    #define TASK_3_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 4)
    #ifndef TASK_4_FUNCTION_NAME
        #error TASK_4_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_4_STACK_SIZE
        #error TASK_4_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_4_PRIORITY
        #error TASK_4_PRIORITY is not defined.
    #endif
    #if ((TASK_4_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_4_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_4_PRIORITY is out of range.
    #endif
    #if ((TASK_4_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_4_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_4_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_4_PRIORITY == TASK_3_PRIORITY))
        #error TASK_4_PRIORITY level has previously been assigned.
    #endif
    #define TASK_4_STACK_START    TASK_3_STACK_START + TASK_3_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_4_FUNCTION_NAME);
#elif defined(TASK_4_FUNCTION_NAME) || defined(TASK_4_STACK_SIZE) || defined(TASK_4_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_4_STACK_START    TASK_3_STACK_START + TASK_3_STACK_SIZE
    #define TASK_4_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 5)
    #ifndef TASK_5_FUNCTION_NAME
        #error TASK_5_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_5_STACK_SIZE
        #error TASK_5_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_5_PRIORITY
        #error TASK_5_PRIORITY is not defined.
    #endif
    #if ((TASK_5_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_5_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_5_PRIORITY is out of range.
    #endif
    #if ((TASK_5_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_5_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_5_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_5_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_5_PRIORITY == TASK_4_PRIORITY))
        #error TASK_5_PRIORITY level has previously been assigned.
    #endif
    #define TASK_5_STACK_START    TASK_4_STACK_START + TASK_4_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_5_FUNCTION_NAME);
#elif defined(TASK_5_FUNCTION_NAME) || defined(TASK_5_STACK_SIZE) || defined(TASK_5_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_5_STACK_START    TASK_4_STACK_START + TASK_4_STACK_SIZE
    #define TASK_5_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 6)
    #ifndef TASK_6_FUNCTION_NAME
        #error TASK_6_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_6_STACK_SIZE
        #error TASK_6_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_6_PRIORITY
        #error TASK_6_PRIORITY is not defined.
    #endif
    #if ((TASK_6_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_6_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_6_PRIORITY is out of range.
    #endif
    #if ((TASK_6_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_6_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_6_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_6_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_6_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_6_PRIORITY == TASK_5_PRIORITY))
        #error TASK_6_PRIORITY level has previously been assigned.
    #endif
    #define TASK_6_STACK_START    TASK_5_STACK_START + TASK_5_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_6_FUNCTION_NAME);
#elif defined(TASK_6_FUNCTION_NAME) || defined(TASK_6_STACK_SIZE) || defined(TASK_6_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_6_STACK_START    TASK_5_STACK_START + TASK_5_STACK_SIZE
    #define TASK_6_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 7)
    #ifndef TASK_7_FUNCTION_NAME
        #error TASK_7_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_7_STACK_SIZE
        #error TASK_7_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_7_PRIORITY
        #error TASK_7_PRIORITY is not defined.
    #endif
    #if ((TASK_7_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_7_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_7_PRIORITY is out of range.
    #endif
    #if ((TASK_7_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_7_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_7_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_7_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_7_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_7_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_7_PRIORITY == TASK_6_PRIORITY))
        #error TASK_7_PRIORITY level has previously been assigned.
    #endif
    #define TASK_7_STACK_START    TASK_6_STACK_START + TASK_6_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_7_FUNCTION_NAME);
#elif defined(TASK_7_FUNCTION_NAME) || defined(TASK_7_STACK_SIZE) || defined(TASK_7_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_7_STACK_START    TASK_6_STACK_START + TASK_6_STACK_SIZE
    #define TASK_7_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 8)
    #ifndef TASK_8_FUNCTION_NAME
        #error TASK_8_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_8_STACK_SIZE
        #error TASK_8_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_8_PRIORITY
        #error TASK_8_PRIORITY is not defined.
    #endif
    #if ((TASK_8_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_8_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_8_PRIORITY is out of range.
    #endif
    #if ((TASK_8_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_8_PRIORITY == TASK_7_PRIORITY))
        #error TASK_8_PRIORITY level has previously been assigned.
    #endif
    #define TASK_8_STACK_START    TASK_7_STACK_START + TASK_7_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_8_FUNCTION_NAME);
#elif defined(TASK_8_FUNCTION_NAME) || defined(TASK_8_STACK_SIZE) || defined(TASK_8_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_8_STACK_START    TASK_7_STACK_START + TASK_7_STACK_SIZE
    #define TASK_8_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 9)
    #ifndef TASK_9_FUNCTION_NAME
        #error TASK_9_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_9_STACK_SIZE
        #error TASK_9_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_9_PRIORITY
        #error TASK_9_PRIORITY is not defined.
    #endif
    #if ((TASK_9_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_9_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_9_PRIORITY is out of range.
    #endif
    #if ((TASK_9_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_9_PRIORITY == TASK_8_PRIORITY))
        #error TASK_9_PRIORITY level has previously been assigned.
    #endif
    #define TASK_9_STACK_START    TASK_8_STACK_START + TASK_8_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_9_FUNCTION_NAME);
#elif defined(TASK_9_FUNCTION_NAME) || defined(TASK_9_STACK_SIZE) || defined(TASK_9_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_9_STACK_START    TASK_8_STACK_START + TASK_8_STACK_SIZE
    #define TASK_9_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 10)
    #ifndef TASK_10_FUNCTION_NAME
        #error TASK_10_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_10_STACK_SIZE
        #error TASK_10_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_10_PRIORITY
        #error TASK_10_PRIORITY is not defined.
    #endif
    #if ((TASK_10_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_10_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_10_PRIORITY is out of range.
    #endif
    #if ((TASK_10_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_10_PRIORITY == TASK_9_PRIORITY))
        #error TASK_10_PRIORITY level has previously been assigned.
    #endif
    #define TASK_10_STACK_START    TASK_9_STACK_START + TASK_9_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_10_FUNCTION_NAME);
#elif defined(TASK_10_FUNCTION_NAME) || defined(TASK_10_STACK_SIZE) || defined(TASK_10_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_10_STACK_START    TASK_9_STACK_START + TASK_9_STACK_SIZE
    #define TASK_10_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 11)
    #ifndef TASK_11_FUNCTION_NAME
        #error TASK_11_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_11_STACK_SIZE
        #error TASK_11_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_11_PRIORITY
        #error TASK_11_PRIORITY is not defined.
    #endif
    #if ((TASK_11_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_11_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_11_PRIORITY is out of range.
    #endif
    #if ((TASK_11_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_11_PRIORITY == TASK_10_PRIORITY)) 
        #error TASK_11_PRIORITY level has previously been assigned.
    #endif
    #define TASK_11_STACK_START    TASK_10_STACK_START + TASK_10_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_11_FUNCTION_NAME);
#elif defined(TASK_11_FUNCTION_NAME) || defined(TASK_11_STACK_SIZE) || defined(TASK_11_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_11_STACK_START    TASK_10_STACK_START + TASK_10_STACK_SIZE
    #define TASK_11_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 12)
    #ifndef TASK_12_FUNCTION_NAME
        #error TASK_12_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_12_STACK_SIZE
        #error TASK_12_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_12_PRIORITY
        #error TASK_12_PRIORITY is not defined.
    #endif
    #if ((TASK_12_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_12_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_12_PRIORITY is out of range.
    #endif
    #if ((TASK_12_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_12_PRIORITY == TASK_11_PRIORITY)) 
        #error TASK_12_PRIORITY level has previously been assigned.
    #endif
    #define TASK_12_STACK_START    TASK_11_STACK_START + TASK_11_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_12_FUNCTION_NAME);
#elif defined(TASK_12_FUNCTION_NAME) || defined(TASK_12_STACK_SIZE) || defined(TASK_12_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_12_STACK_START    TASK_11_STACK_START + TASK_11_STACK_SIZE
    #define TASK_12_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 13)
    #ifndef TASK_13_FUNCTION_NAME
        #error TASK_13_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_13_STACK_SIZE
        #error TASK_13_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_13_PRIORITY
        #error TASK_13_PRIORITY is not defined.
    #endif
    #if ((TASK_13_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_13_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_13_PRIORITY is out of range.
    #endif
    #if ((TASK_13_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_13_PRIORITY == TASK_12_PRIORITY))
        #error TASK_13_PRIORITY level has previously been assigned.
    #endif
    #define TASK_13_STACK_START    TASK_12_STACK_START + TASK_12_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_13_FUNCTION_NAME);
#elif defined(TASK_13_FUNCTION_NAME) || defined(TASK_13_STACK_SIZE) || defined(TASK_13_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_13_STACK_START    TASK_12_STACK_START + TASK_12_STACK_SIZE
    #define TASK_13_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 14)
    #ifndef TASK_14_FUNCTION_NAME
        #error TASK_14_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_14_STACK_SIZE
        #error TASK_14_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_14_PRIORITY
        #error TASK_14_PRIORITY is not defined.
    #endif
    #if ((TASK_14_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_14_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_14_PRIORITY is out of range.
    #endif
    #if ((TASK_14_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_14_PRIORITY == TASK_13_PRIORITY))
        #error TASK_14_PRIORITY level has previously been assigned.
    #endif
    #define TASK_14_STACK_START    TASK_13_STACK_START + TASK_13_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_14_FUNCTION_NAME);
#elif defined(TASK_14_FUNCTION_NAME) || defined(TASK_14_STACK_SIZE) || defined(TASK_14_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_14_STACK_START    TASK_13_STACK_START + TASK_13_STACK_SIZE
    #define TASK_14_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 15)
    #ifndef TASK_15_FUNCTION_NAME
        #error TASK_15_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_15_STACK_SIZE
        #error TASK_15_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_15_PRIORITY
        #error TASK_15_PRIORITY is not defined.
    #endif
    #if ((TASK_15_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_15_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_15_PRIORITY is out of range.
    #endif
    #if ((TASK_15_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_15_PRIORITY == TASK_14_PRIORITY))
        #error TASK_15_PRIORITY level has previously been assigned.
    #endif
    #define TASK_15_STACK_START    TASK_14_STACK_START + TASK_14_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_15_FUNCTION_NAME);
#elif defined(TASK_15_FUNCTION_NAME) || defined(TASK_15_STACK_SIZE) || defined(TASK_15_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_15_STACK_START    TASK_14_STACK_START + TASK_14_STACK_SIZE
    #define TASK_15_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 16)
    #ifndef TASK_16_FUNCTION_NAME
        #error TASK_16_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_16_STACK_SIZE
        #error TASK_16_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_16_PRIORITY
        #error TASK_16_PRIORITY is not defined.
    #endif
    #if ((TASK_16_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_16_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_16_PRIORITY is out of range.
    #endif
    #if ((TASK_16_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_16_PRIORITY == TASK_15_PRIORITY))
        #error TASK_16_PRIORITY level has previously been assigned.
    #endif
    #define TASK_16_STACK_START    TASK_15_STACK_START + TASK_15_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_16_FUNCTION_NAME);
#elif defined(TASK_16_FUNCTION_NAME) || defined(TASK_16_STACK_SIZE) || defined(TASK_16_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_16_STACK_START    TASK_15_STACK_START + TASK_15_STACK_SIZE
    #define TASK_16_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 17)
    #ifndef TASK_17_FUNCTION_NAME
        #error TASK_17_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_17_STACK_SIZE
        #error TASK_17_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_17_PRIORITY
        #error TASK_17_PRIORITY is not defined.
    #endif
    #if ((TASK_17_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_17_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_17_PRIORITY is out of range.
    #endif
    #if ((TASK_17_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_17_PRIORITY == TASK_16_PRIORITY))
        #error TASK_17_PRIORITY level has previously been assigned.
    #endif
    #define TASK_17_STACK_START    TASK_16_STACK_START + TASK_16_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_17_FUNCTION_NAME);
#elif defined(TASK_17_FUNCTION_NAME) || defined(TASK_17_STACK_SIZE) || defined(TASK_17_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_17_STACK_START    TASK_16_STACK_START + TASK_16_STACK_SIZE
    #define TASK_17_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 18)
    #ifndef TASK_18_FUNCTION_NAME
        #error TASK_18_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_18_STACK_SIZE
        #error TASK_18_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_18_PRIORITY
        #error TASK_18_PRIORITY is not defined.
    #endif
    #if ((TASK_18_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_18_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_18_PRIORITY is out of range.
    #endif
    #if ((TASK_18_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_18_PRIORITY == TASK_17_PRIORITY))
        #error TASK_18_PRIORITY level has previously been assigned.
    #endif
    #define TASK_18_STACK_START    TASK_17_STACK_START + TASK_17_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_18_FUNCTION_NAME);
#elif defined(TASK_18_FUNCTION_NAME) || defined(TASK_18_STACK_SIZE) || defined(TASK_18_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_18_STACK_START    TASK_17_STACK_START + TASK_17_STACK_SIZE
    #define TASK_18_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 19)
    #ifndef TASK_19_FUNCTION_NAME
        #error TASK_19_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_19_STACK_SIZE
        #error TASK_19_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_19_PRIORITY
        #error TASK_19_PRIORITY is not defined.
    #endif
    #if ((TASK_19_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_19_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_19_PRIORITY is out of range.
    #endif
    #if ((TASK_19_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_19_PRIORITY == TASK_18_PRIORITY))
        #error TASK_19_PRIORITY level has previously been assigned.
    #endif
    #define TASK_19_STACK_START    TASK_18_STACK_START + TASK_18_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_19_FUNCTION_NAME);
#elif defined(TASK_19_FUNCTION_NAME) || defined(TASK_19_STACK_SIZE) || defined(TASK_19_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_19_STACK_START    TASK_18_STACK_START + TASK_18_STACK_SIZE
    #define TASK_19_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 20)
    #ifndef TASK_20_FUNCTION_NAME
        #error TASK_20_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_20_STACK_SIZE
        #error TASK_20_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_20_PRIORITY
        #error TASK_20_PRIORITY is not defined.
    #endif
    #if ((TASK_20_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_20_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_20_PRIORITY is out of range.
    #endif
    #if ((TASK_20_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_20_PRIORITY == TASK_19_PRIORITY))
        #error TASK_20_PRIORITY level has previously been assigned.
    #endif
    #define TASK_20_STACK_START    TASK_19_STACK_START + TASK_19_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_20_FUNCTION_NAME);
#elif defined(TASK_20_FUNCTION_NAME) || defined(TASK_20_STACK_SIZE) || defined(TASK_20_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_20_STACK_START    TASK_19_STACK_START + TASK_19_STACK_SIZE
    #define TASK_20_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 21)
    #ifndef TASK_21_FUNCTION_NAME
        #error TASK_21_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_21_STACK_SIZE
        #error TASK_21_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_21_PRIORITY
        #error TASK_21_PRIORITY is not defined.
    #endif
    #if ((TASK_21_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_21_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_21_PRIORITY is out of range.
    #endif
    #if ((TASK_21_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_21_PRIORITY == TASK_20_PRIORITY))
        #error TASK_21_PRIORITY level has previously been assigned.
    #endif
    #define TASK_21_STACK_START    TASK_20_STACK_START + TASK_20_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_21_FUNCTION_NAME);
#elif defined(TASK_21_FUNCTION_NAME) || defined(TASK_21_STACK_SIZE) || defined(TASK_21_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_21_STACK_START    TASK_20_STACK_START + TASK_20_STACK_SIZE
    #define TASK_21_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 22)
    #ifndef TASK_22_FUNCTION_NAME
        #error TASK_22_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_22_STACK_SIZE
        #error TASK_22_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_22_PRIORITY
        #error TASK_22_PRIORITY is not defined.
    #endif
    #if ((TASK_22_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_22_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_22_PRIORITY is out of range.
    #endif
    #if ((TASK_22_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_22_PRIORITY == TASK_21_PRIORITY))
        #error TASK_22_PRIORITY level has previously been assigned.
    #endif
    #define TASK_22_STACK_START    TASK_21_STACK_START + TASK_21_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_22_FUNCTION_NAME);
#elif defined(TASK_22_FUNCTION_NAME) || defined(TASK_22_STACK_SIZE) || defined(TASK_22_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_22_STACK_START    TASK_21_STACK_START + TASK_21_STACK_SIZE
    #define TASK_22_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 23)
    #ifndef TASK_23_FUNCTION_NAME
        #error TASK_23_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_23_STACK_SIZE
        #error TASK_23_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_23_PRIORITY
        #error TASK_23_PRIORITY is not defined.
    #endif
    #if ((TASK_23_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_23_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_23_PRIORITY is out of range.
    #endif
    #if ((TASK_23_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_23_PRIORITY == TASK_22_PRIORITY))
        #error TASK_23_PRIORITY level has previously been assigned.
    #endif
    #define TASK_23_STACK_START    TASK_22_STACK_START + TASK_22_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_23_FUNCTION_NAME);
#elif defined(TASK_23_FUNCTION_NAME) || defined(TASK_23_STACK_SIZE) || defined(TASK_23_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_23_STACK_START    TASK_22_STACK_START + TASK_22_STACK_SIZE
    #define TASK_23_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 24)
    #ifndef TASK_24_FUNCTION_NAME
        #error TASK_24_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_24_STACK_SIZE
        #error TASK_24_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_24_PRIORITY
        #error TASK_24_PRIORITY is not defined.
    #endif
    #if ((TASK_24_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_24_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_24_PRIORITY is out of range.
    #endif
    #if ((TASK_24_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_24_PRIORITY == TASK_23_PRIORITY))
        #error TASK_24_PRIORITY level has previously been assigned.
    #endif
    #define TASK_24_STACK_START    TASK_23_STACK_START + TASK_23_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_24_FUNCTION_NAME);
#elif defined(TASK_24_FUNCTION_NAME) || defined(TASK_24_STACK_SIZE) || defined(TASK_24_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_24_STACK_START    TASK_23_STACK_START + TASK_23_STACK_SIZE
    #define TASK_24_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 25)
    #ifndef TASK_25_FUNCTION_NAME
        #error TASK_25_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_25_STACK_SIZE
        #error TASK_25_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_25_PRIORITY
        #error TASK_25_PRIORITY is not defined.
    #endif
    #if ((TASK_25_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_25_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_25_PRIORITY is out of range.
    #endif
    #if ((TASK_25_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_25_PRIORITY == TASK_24_PRIORITY))
        #error TASK_25_PRIORITY level has previously been assigned.
    #endif
    #define TASK_25_STACK_START    TASK_24_STACK_START + TASK_24_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_25_FUNCTION_NAME);
#elif defined(TASK_25_FUNCTION_NAME) || defined(TASK_25_STACK_SIZE) || defined(TASK_25_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_25_STACK_START    TASK_24_STACK_START + TASK_24_STACK_SIZE
    #define TASK_25_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 26)
    #ifndef TASK_26_FUNCTION_NAME
        #error TASK_26_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_26_STACK_SIZE
        #error TASK_26_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_26_PRIORITY
        #error TASK_26_PRIORITY is not defined.
    #endif
    #if ((TASK_26_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_26_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_26_PRIORITY is out of range.
    #endif
    #if ((TASK_26_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_24_PRIORITY) || \
         (TASK_26_PRIORITY == TASK_25_PRIORITY))
        #error TASK_26_PRIORITY level has previously been assigned.
    #endif
    #define TASK_26_STACK_START    TASK_25_STACK_START + TASK_25_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_26_FUNCTION_NAME);
#elif defined(TASK_26_FUNCTION_NAME) || defined(TASK_26_STACK_SIZE) || defined(TASK_26_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_26_STACK_START    TASK_25_STACK_START + TASK_25_STACK_SIZE
    #define TASK_26_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 27)
    #ifndef TASK_27_FUNCTION_NAME
        #error TASK_27_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_27_STACK_SIZE
        #error TASK_27_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_27_PRIORITY
        #error TASK_27_PRIORITY is not defined.
    #endif
    #if ((TASK_27_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_27_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_27_PRIORITY is out of range.
    #endif
    #if ((TASK_27_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_24_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_25_PRIORITY) || \
         (TASK_27_PRIORITY == TASK_26_PRIORITY))
        #error TASK_27_PRIORITY level has previously been assigned.
    #endif
    #define TASK_27_STACK_START    TASK_26_STACK_START + TASK_26_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_27_FUNCTION_NAME);
#elif defined(TASK_27_FUNCTION_NAME) || defined(TASK_27_STACK_SIZE) || defined(TASK_27_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_27_STACK_START    TASK_26_STACK_START + TASK_26_STACK_SIZE
    #define TASK_27_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 28)
    #ifndef TASK_28_FUNCTION_NAME
        #error TASK_28_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_28_STACK_SIZE
        #error TASK_28_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_28_PRIORITY
        #error TASK_28_PRIORITY is not defined.
    #endif
    #if ((TASK_28_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_28_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_28_PRIORITY is out of range.
    #endif
    #if ((TASK_28_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_24_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_25_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_26_PRIORITY) || \
         (TASK_28_PRIORITY == TASK_27_PRIORITY))
        #error TASK_28_PRIORITY level has previously been assigned.
    #endif
    #define TASK_28_STACK_START    TASK_27_STACK_START + TASK_27_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_28_FUNCTION_NAME);
#elif defined(TASK_28_FUNCTION_NAME) || defined(TASK_28_STACK_SIZE) || defined(TASK_28_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_28_STACK_START    TASK_27_STACK_START + TASK_27_STACK_SIZE
    #define TASK_28_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 29)
    #ifndef TASK_29_FUNCTION_NAME
        #error TASK_29_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_29_STACK_SIZE
        #error TASK_29_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_29_PRIORITY
        #error TASK_29_PRIORITY is not defined.
    #endif
    #if ((TASK_29_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_29_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_29_PRIORITY is out of range.
    #endif
    #if ((TASK_29_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_24_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_25_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_26_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_27_PRIORITY) || \
         (TASK_29_PRIORITY == TASK_28_PRIORITY))
        #error TASK_29_PRIORITY level has previously been assigned.
    #endif
    #define TASK_29_STACK_START    TASK_28_STACK_START + TASK_28_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_29_FUNCTION_NAME);
#elif defined(TASK_29_FUNCTION_NAME) || defined(TASK_29_STACK_SIZE) || defined(TASK_29_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_29_STACK_START    TASK_28_STACK_START + TASK_28_STACK_SIZE
    #define TASK_29_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 30)
    #ifndef TASK_30_FUNCTION_NAME
        #error TASK_30_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_30_STACK_SIZE
        #error TASK_30_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_30_PRIORITY
        #error TASK_30_PRIORITY is not defined.
    #endif
    #if ((TASK_30_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_30_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_30_PRIORITY is out of range.
    #endif
    #if ((TASK_30_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_24_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_25_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_26_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_27_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_28_PRIORITY) || \
         (TASK_30_PRIORITY == TASK_29_PRIORITY))
        #error TASK_30_PRIORITY level has previously been assigned.
    #endif
    #define TASK_30_STACK_START    TASK_29_STACK_START + TASK_29_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_30_FUNCTION_NAME);
#elif defined(TASK_30_FUNCTION_NAME) || defined(TASK_30_STACK_SIZE) || defined(TASK_30_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_30_STACK_START    TASK_29_STACK_START + TASK_29_STACK_SIZE
    #define TASK_30_STACK_SIZE    0
#endif    

#if (NUM_TASKS >= 31)
    #ifndef TASK_31_FUNCTION_NAME
        #error TASK_31_FUNCTION_NAME is not defined.
    #endif
    #ifndef TASK_31_STACK_SIZE
        #error TASK_31_STACK_SIZE is not defined.
    #endif
    #ifndef TASK_31_PRIORITY
        #error TASK_31_PRIORITY is not defined.
    #endif
    #if ((TASK_31_PRIORITY < TASK_PRIORITY_MIN_NUM) || (TASK_31_PRIORITY > TASK_PRIORITY_MAX_NUM))
        #error TASK_31_PRIORITY is out of range.
    #endif
    #if ((TASK_31_PRIORITY == TASK_0_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_1_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_2_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_3_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_4_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_5_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_6_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_7_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_8_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_9_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_10_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_11_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_12_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_13_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_14_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_15_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_16_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_17_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_18_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_19_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_20_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_21_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_22_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_23_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_24_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_25_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_26_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_27_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_28_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_29_PRIORITY) || \
         (TASK_31_PRIORITY == TASK_30_PRIORITY))
        #error TASK_31_PRIORITY level has previously been assigned.
    #endif
    #define TASK_31_STACK_START    TASK_30_STACK_START + TASK_30_STACK_SIZE + TASK_STACK_SIZE_MIN
    TASK_GEN_PROTOTYPE(TASK_31_FUNCTION_NAME);
#elif defined(TASK_31_FUNCTION_NAME) || defined(TASK_31_STACK_SIZE) || defined(TASK_31_PRIORITY)
    #error NUM_TASKS is too small to accomodate all defined tasks.
#else
    #define TASK_31_STACK_START    TASK_30_STACK_START + TASK_30_STACK_SIZE
    #define TASK_31_STACK_SIZE    0
#endif    


#define RTOS_STACKS_SIZE        (TASK_31_STACK_START + TASK_31_STACK_SIZE + TASK_STACK_SIZE_MIN)

#if (RTOS_STACKS_SIZE > 0)
	uint32  rtosstacks[RTOS_STACKS_SIZE] __attribute__((aligned(4)));
	const uint32 rtosstacks_sizeof = (RTOS_STACKS_SIZE);
#else
	uint32  rtosstacks[1] __attribute__((aligned(4)));
	const uint32 rtosstacks_sizeof = (1);
#endif

// RTOS ROM_TCB structure:
const uint8 task_rom_list_sizeof = (NUM_TASKS + 1);
const ROM_TCB task_rom_list[] =
    {                                           
        {TASK_0_FUNCTION_NAME,0,0,TASK_0_PRIORITY}     
#if (NUM_TASKS >= 1)
        ,{TASK_1_FUNCTION_NAME,&rtosstacks[TASK_1_STACK_START+TASK_1_STACK_SIZE+TASK_STACK_SIZE_MIN],1,TASK_1_PRIORITY}     
#endif
#if (NUM_TASKS >= 2)
        ,{TASK_2_FUNCTION_NAME,&rtosstacks[TASK_2_STACK_START+TASK_2_STACK_SIZE+TASK_STACK_SIZE_MIN],2,TASK_2_PRIORITY}     
#endif
#if (NUM_TASKS >= 3)
        ,{TASK_3_FUNCTION_NAME,&rtosstacks[TASK_3_STACK_START+TASK_3_STACK_SIZE+TASK_STACK_SIZE_MIN],3,TASK_3_PRIORITY}     
#endif
#if (NUM_TASKS >= 4)
        ,{TASK_4_FUNCTION_NAME,&rtosstacks[TASK_4_STACK_START+TASK_4_STACK_SIZE+TASK_STACK_SIZE_MIN],4,TASK_4_PRIORITY}     
#endif
#if (NUM_TASKS >= 5)
        ,{TASK_5_FUNCTION_NAME,&rtosstacks[TASK_5_STACK_START+TASK_5_STACK_SIZE+TASK_STACK_SIZE_MIN],5,TASK_5_PRIORITY}     
#endif
#if (NUM_TASKS >= 6)
        ,{TASK_6_FUNCTION_NAME,&rtosstacks[TASK_6_STACK_START+TASK_6_STACK_SIZE+TASK_STACK_SIZE_MIN],6,TASK_6_PRIORITY}     
#endif
#if (NUM_TASKS >= 7)
        ,{TASK_7_FUNCTION_NAME,&rtosstacks[TASK_7_STACK_START+TASK_7_STACK_SIZE+TASK_STACK_SIZE_MIN],7,TASK_7_PRIORITY}     
#endif
#if (NUM_TASKS >= 8)
        ,{TASK_8_FUNCTION_NAME,&rtosstacks[TASK_8_STACK_START+TASK_8_STACK_SIZE+TASK_STACK_SIZE_MIN],8,TASK_8_PRIORITY}     
#endif
#if (NUM_TASKS >= 9)
        ,{TASK_9_FUNCTION_NAME,&rtosstacks[TASK_9_STACK_START+TASK_9_STACK_SIZE+TASK_STACK_SIZE_MIN],9,TASK_9_PRIORITY}  
#endif
#if (NUM_TASKS >= 10)
        ,{TASK_10_FUNCTION_NAME,&rtosstacks[TASK_10_STACK_START+TASK_10_STACK_SIZE+TASK_STACK_SIZE_MIN],10,TASK_10_PRIORITY}     
#endif
#if (NUM_TASKS >= 11)
        ,{TASK_11_FUNCTION_NAME,&rtosstacks[TASK_11_STACK_START+TASK_11_STACK_SIZE+TASK_STACK_SIZE_MIN],11,TASK_11_PRIORITY}     
#endif
#if (NUM_TASKS >= 12)
        ,{TASK_12_FUNCTION_NAME,&rtosstacks[TASK_12_STACK_START+TASK_12_STACK_SIZE+TASK_STACK_SIZE_MIN],12,TASK_12_PRIORITY}     
#endif
#if (NUM_TASKS >= 13)
        ,{TASK_13_FUNCTION_NAME,&rtosstacks[TASK_13_STACK_START+TASK_13_STACK_SIZE+TASK_STACK_SIZE_MIN],13,TASK_13_PRIORITY}     
#endif
#if (NUM_TASKS >= 14)
        ,{TASK_14_FUNCTION_NAME,&rtosstacks[TASK_14_STACK_START+TASK_14_STACK_SIZE+TASK_STACK_SIZE_MIN],14,TASK_14_PRIORITY}     
#endif
#if (NUM_TASKS >= 15)
        ,{TASK_15_FUNCTION_NAME,&rtosstacks[TASK_15_STACK_START+TASK_15_STACK_SIZE+TASK_STACK_SIZE_MIN],15,TASK_15_PRIORITY}     
#endif
#if (NUM_TASKS >= 16)
        ,{TASK_16_FUNCTION_NAME,&rtosstacks[TASK_16_STACK_START+TASK_16_STACK_SIZE+TASK_STACK_SIZE_MIN],16,TASK_16_PRIORITY}     
#endif
#if (NUM_TASKS >= 17)
        ,{TASK_17_FUNCTION_NAME,&rtosstacks[TASK_17_STACK_START+TASK_17_STACK_SIZE+TASK_STACK_SIZE_MIN],17,TASK_17_PRIORITY}     
#endif
#if (NUM_TASKS >= 18)
        ,{TASK_18_FUNCTION_NAME,&rtosstacks[TASK_18_STACK_START+TASK_18_STACK_SIZE+TASK_STACK_SIZE_MIN],18,TASK_18_PRIORITY}     
#endif
#if (NUM_TASKS >= 19)
        ,{TASK_19_FUNCTION_NAME,&rtosstacks[TASK_19_STACK_START+TASK_19_STACK_SIZE+TASK_STACK_SIZE_MIN],19,TASK_19_PRIORITY}     
#endif
#if (NUM_TASKS >= 20)
        ,{TASK_20_FUNCTION_NAME,&rtosstacks[TASK_20_STACK_START+TASK_20_STACK_SIZE+TASK_STACK_SIZE_MIN],20,TASK_20_PRIORITY}     
#endif
#if (NUM_TASKS >= 21)
        ,{TASK_21_FUNCTION_NAME,&rtosstacks[TASK_21_STACK_START+TASK_21_STACK_SIZE+TASK_STACK_SIZE_MIN],21,TASK_21_PRIORITY}     
#endif
#if (NUM_TASKS >= 22)
        ,{TASK_22_FUNCTION_NAME,&rtosstacks[TASK_22_STACK_START+TASK_22_STACK_SIZE+TASK_STACK_SIZE_MIN],22,TASK_22_PRIORITY}     
#endif
#if (NUM_TASKS >= 23)
        ,{TASK_23_FUNCTION_NAME,&rtosstacks[TASK_23_STACK_START+TASK_23_STACK_SIZE+TASK_STACK_SIZE_MIN],23,TASK_23_PRIORITY}     
#endif
#if (NUM_TASKS >= 24)
        ,{TASK_24_FUNCTION_NAME,&rtosstacks[TASK_24_STACK_START+TASK_24_STACK_SIZE+TASK_STACK_SIZE_MIN],24,TASK_24_PRIORITY}     
#endif
#if (NUM_TASKS >= 25)
        ,{TASK_25_FUNCTION_NAME,&rtosstacks[TASK_25_STACK_START+TASK_25_STACK_SIZE+TASK_STACK_SIZE_MIN],25,TASK_25_PRIORITY}     
#endif
#if (NUM_TASKS >= 26)
        ,{TASK_26_FUNCTION_NAME,&rtosstacks[TASK_26_STACK_START+TASK_26_STACK_SIZE+TASK_STACK_SIZE_MIN],26,TASK_26_PRIORITY}     
#endif
#if (NUM_TASKS >= 27)
        ,{TASK_27_FUNCTION_NAME,&rtosstacks[TASK_27_STACK_START+TASK_27_STACK_SIZE+TASK_STACK_SIZE_MIN],27,TASK_27_PRIORITY}     
#endif
#if (NUM_TASKS >= 28)
        ,{TASK_28_FUNCTION_NAME,&rtosstacks[TASK_28_STACK_START+TASK_28_STACK_SIZE+TASK_STACK_SIZE_MIN],28,TASK_28_PRIORITY}     
#endif
#if (NUM_TASKS >= 29)
        ,{TASK_29_FUNCTION_NAME,&rtosstacks[TASK_29_STACK_START+TASK_29_STACK_SIZE+TASK_STACK_SIZE_MIN],29,TASK_29_PRIORITY}     
#endif
#if (NUM_TASKS >= 30)
        ,{TASK_30_FUNCTION_NAME,&rtosstacks[TASK_30_STACK_START+TASK_30_STACK_SIZE+TASK_STACK_SIZE_MIN],30,TASK_30_PRIORITY}     
#endif
#if (NUM_TASKS >= 31)
        ,{TASK_31_FUNCTION_NAME,&rtosstacks[TASK_31_STACK_START+TASK_31_STACK_SIZE+TASK_STACK_SIZE_MIN],31,TASK_31_PRIORITY}
#endif
    };

