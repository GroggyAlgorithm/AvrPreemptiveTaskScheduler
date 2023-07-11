/**
 * \file PreemptiveTaskSchedulerConfig.h
 * \author: Tim Robbins
 * \brief Configuration file for preemptive task scheduling and concurrent functionality. \n
 */ 
#ifndef __PREEMPTIVETASKSCHEDULERCONFIG_H___
#define __PREEMPTIVETASKSCHEDULERCONFIG_H___	1




#include <avr/io.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */



///Maximum Amount Of Tasks Allowed
#ifndef MAX_TASKS
#define MAX_TASKS				11
#endif

///The amount of registers in the task general purpose file register
#ifndef TASK_REGISTERS
#define TASK_REGISTERS			32
#endif

///The amount of ticks for our interrupt scheduler
#ifndef TASK_INTERRUPT_TICKS
#define	TASK_INTERRUPT_TICKS	0x2f0
#endif


///The interrupt vector for our scheduler interrupt
#ifndef SCHEDULER_INT_VECTOR
#define SCHEDULER_INT_VECTOR	TIMER3_OVF_vect
#endif

///Our task stack size
#ifndef TASK_STACK_SIZE
#define TASK_STACK_SIZE			64
#endif

///Main keyword for interrupts (ex. ISR for AVR)
#ifndef SCHEDULER_INTERRUPT_KEYWORD
#define SCHEDULER_INTERRUPT_KEYWORD		ISR
#endif






#if SCHEDULER_INT_VECTOR == TIMER3_OVF_vect
#ifndef _SCHEDULER_STOP_TICK
#define _SCHEDULER_STOP_TICK()		TCCR3B &= ~(1 << CS30 | 1 << CS31 | 1 << CS32)
#endif
#ifndef _SCHEDULER_START_TICK
#define _SCHEDULER_START_TICK()		TCCR3B |= (1 << CS30)
#endif
#ifndef _SCHEDULER_LOAD_ISR_REG
#define _SCHEDULER_LOAD_ISR_REG()	TCNT3 = 0xffff-TASK_INTERRUPT_TICKS
#endif
#ifndef _SCHEDULER_EN_ISR
#define _SCHEDULER_EN_ISR()			TIMSK3 |= (1 << TOIE3)
#endif
#elif SCHEDULER_INT_VECTOR == TIMER2_OVF_vect
#ifndef _SCHEDULER_STOP_TICK
#define _SCHEDULER_STOP_TICK()		TCCR2B &= ~(1 << CS20 | 1 << CS21 | 1 << CS22)
#endif
#ifndef _SCHEDULER_START_TICK
#define _SCHEDULER_START_TICK()		TCCR2B |= (1 << CS20 | 1 << CS22)
#endif
#ifndef _SCHEDULER_LOAD_ISR_REG
#define _SCHEDULER_LOAD_ISR_REG()	TCNT2 = 0xff-TASK_INTERRUPT_TICKS
#endif
#ifndef _SCHEDULER_EN_ISR
#define _SCHEDULER_EN_ISR()			TIMSK2 |= (1 << TOIE2)
#endif
#elif SCHEDULER_INT_VECTOR == TIMER1_OVF_vect
#ifndef _SCHEDULER_STOP_TICK
#define _SCHEDULER_STOP_TICK()		TCCR1B &= ~(1 << CS10 | 1 << CS11 | 1 << CS12)
#endif
#ifndef _SCHEDULER_START_TICK
#define _SCHEDULER_START_TICK()		TCCR1B |= (1 << CS10 )
#endif
#ifndef _SCHEDULER_LOAD_ISR_REG
#define _SCHEDULER_LOAD_ISR_REG()	TCNT1 = 0xffff-TASK_INTERRUPT_TICKS
#endif
#ifndef _SCHEDULER_EN_ISR
#define _SCHEDULER_EN_ISR()			TIMSK1 |= (1 << TOIE1)
#endif
#elif SCHEDULER_INT_VECTOR == TIMER0_OVF_vect
#ifndef _SCHEDULER_STOP_TICK
#define _SCHEDULER_STOP_TICK()		TCCR0B &= ~(1 << CS00 | 1 << CS01 | 1 << CS02)
#endif
#ifndef _SCHEDULER_START_TICK
#define _SCHEDULER_START_TICK()		TCCR0B |= (1 << CS00 | 1 << CS02)
#endif
#ifndef _SCHEDULER_LOAD_ISR_REG
#define _SCHEDULER_LOAD_ISR_REG()	TCNT0 = 0xff-TASK_INTERRUPT_TICKS
#endif
#ifndef _SCHEDULER_EN_ISR
#define _SCHEDULER_EN_ISR()			TIMSK0 |= (1 << TOIE0)
#endif

#elif SCHEDULER_INT_VECTOR == WDT_vect
#ifndef _SCHEDULER_STOP_TICK
#define _SCHEDULER_STOP_TICK()
#endif
#ifndef _SCHEDULER_START_TICK
#define _SCHEDULER_START_TICK()
#endif
#ifndef _SCHEDULER_LOAD_ISR_REG
#define _SCHEDULER_LOAD_ISR_REG()	WDTCSR |= 1 << WDIE
#endif
#ifndef _SCHEDULER_EN_ISR
#define _SCHEDULER_EN_ISR()
#endif
#else
#warning ISR Built in not currently supported by task scheduler. You must define your own terms.
#endif


















#define _SCHEDULER_LAUNCH_ISR()		_SCHEDULER_STOP_TICK(); _SCHEDULER_EN_ISR(); _SCHEDULER_LOAD_ISR_REG(); _SCHEDULER_START_TICK()



///The data address for setting a stack start address
#define _TASK_STACK_START_ADDRESS(_v) (((void *)(RAMEND - ((_v)*TASK_STACK_SIZE+sizeof(TaskControl_t)+1) )))










///
#define HIGHEST_TASK_PRIORITY 32700



#ifdef	__cplusplus
}
#endif /* __cplusplus */











#endif /* __PREEMPTIVETASKSCHEDULERCONFIG_H___ */