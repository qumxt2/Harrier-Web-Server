#ifndef CMX_CXVENDOR_H
#define CMX_CXVENDOR_H

/* for Microchip PIC32 processors */
#define PROCESSOR PIC32

#define PROC_DISABLE_INT K_OS_Disable_Interrupts()	/* could be assembly, 
															depends on compiler */
#define PROC_ENABLE_INT K_OS_Enable_Interrupts()	/* could be assembly, 
															depends on compiler */

#define PROC_SAVE_INTERRUPTS K_OS_Save_Interrupts()
#define PROC_RESTORE_INTERRUPTS K_OS_Restore_Interrupts()

#ifdef CMX_INIT_MODULE 

byte int_count;		/* counts depth of interrupts */
byte locked_out;	/* task lock out flag, disables scheduling */
byte cmx_flag1;		/* CMX flag register, internal use */
byte int_mask_holder;		/* holder for interrupt state, internal use */
struct _tcb *activetcb;		/* activetcb pointer */
word32 *stack_holder;	/* stack holder */ 
word32 *interrupt_stack; /* interrupt stack */
byte do_timer_tsk;
byte timer_action;
byte active_priority;

#else

extern byte int_count;
extern byte locked_out;	
extern byte cmx_flag1;
extern byte int_mask_holder;		/* holder for interrupt state, internal use */
extern struct _tcb *activetcb;		/* activetcb pointer */
extern word32 *stack_holder;	/* stack holder */ 
extern word32 *interrupt_stack; /* interrupt stack */
extern byte do_timer_tsk;
extern byte timer_action;
extern byte active_priority;

#endif

/* CMX cmx_flag1 bit representation */
#define preempted 0x01	/* higher priority task ready, or task running going
									to sleep */
#define do_coop_sched 0x10	/* perform a cooperative reschedule, preempted 
										flag over rules this */

#define idle_flag	0x40		/* informs K_I_Scheduler, to test for power down state */
#define cmx_active 0x80		/* informs CMX, that user has entered RTOS */

#define PREEMPTED cmx_flag1 |= preempted
#define TEST_NOT_PREEMPTED !(cmx_flag1 & preempted)
#define DO_COOP_SCHED cmx_flag1 |= do_coop_sched
#define SET_TIMER_ACTIVE timer_action = TRUE
#define CLR_TIMER_ACTIVE timer_action = FALSE
#define TEST_TIMER_ACTIVE timer_action
#define TEST_CMX_ACTIVE cmx_flag1 & cmx_active
#define CMX_ACTIVE cmx_flag1 |= cmx_active

#define DO_TIMER_TSK do_timer_tsk = TRUE

#endif /* CMX_CXVENDOR_H */

