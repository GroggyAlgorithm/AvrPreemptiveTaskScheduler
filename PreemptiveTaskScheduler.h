/**
 * \file PreemptiveTaskScheduler.h
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and concurrent functionality. \n
 *
 * For descriptions of definitions, refer to attached README.md  \n
 *
 * Links for influence and resources: \n
 * https://github.com/arbv/avr-context \n
 * https://github.com/kcuzner/kos-avr + http://kevincuzner.com/2015/12/31/writing-a-preemptive-task-scheduler-for-avr/ \n
 * https://gist.github.com/dheeptuck/da0a347358e60c77ea259090a61d78f4 \n
 * https://medium.com/@dheeptuck/building-a-real-time-operating-system-rtos-ground-up-a70640c64e93 \n
 * https://www.cs.princeton.edu/courses/archive/fall16/cos318/projects/project3/p3.html \n
 * https://www.nongnu.org/avr-libc/user-manual/inline_asm.html \n
 *
 */ 
#ifndef __PREEMPTIVETASKSCHEDULER_H__
#define __PREEMPTIVETASKSCHEDULER_H__	1



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>


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
#define	TASK_INTERRUPT_TICKS	0x2f0//0xffff//0x2F0//0x2FF
#endif


///The interrupt vector for our scheduler interrupt
#ifndef SCHEDULER_INT_VECTOR
#define SCHEDULER_INT_VECTOR	TIMER3_OVF_vect
#endif

///Our max task stack size
#ifndef MAX_TASK_STACK_SIZE
#define MAX_TASK_STACK_SIZE		64
#endif

///Main keyword for interrupts (ex. ISR for AVR)
#ifndef SCHEDULER_INTERRUPT_KEYWORD
#define SCHEDULER_INTERRUPT_KEYWORD ISR
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


//CONSTANTS-------------------------------------------------------------------------------------------



///Constant size of our tasks stack
#if MAX_TASK_STACK_SIZE > 0xFFFFFFFF
const uint64_t TASK_STACK_SIZE = MAX_TASK_STACK_SIZE;
#elif MAX_TASK_STACK_SIZE > 0xFFFF
const uint32_t TASK_STACK_SIZE = MAX_TASK_STACK_SIZE;
#elif MAX_TASK_STACK_SIZE > 0xFF
const uint16_t TASK_STACK_SIZE = MAX_TASK_STACK_SIZE;
#else
const uint8_t TASK_STACK_SIZE  = MAX_TASK_STACK_SIZE;
#endif



/*
	ASM Constants
*/
//Program counter high/low
__asm__(".equ CONTEXT_OFFSET_PC_L, 33 \n\t");
__asm__(".equ CONTEXT_OFFSET_PC_H, 34 \n\t");



//Stack pointer high/low
__asm__(".equ CONTEXT_OFFSET_SP_L, 35 \n\t");
__asm__(".equ CONTEXT_OFFSET_SP_H, 36 \n\t");



//Register back offset
__asm__(".equ CONTEXT_OFFSET_R26,  9 \n\t");



//----------------------------------------------------------------------------------------------------



///The data address for setting a stack start address
#define _TASK_STACK_START_ADDRESS(_v) (((void *)(RAMEND - ((_v)*TASK_STACK_SIZE+sizeof(TaskControl_t)+1) )))



//DATA TYPES------------------------------------------------------------------------------------------



///Data type for our semaphores
typedef int8_t SemaphoreValueType_t;

///Data type for task indices (changeable if looking for higher values)
typedef int8_t TaskIndiceType_t;

///Data type for the register size for our tasks
typedef uint8_t TaskRegisterType_t;

///Data type for the size of our memory locations
typedef uint16_t TaskMemoryLocationType_t;

///Data type for timeouts
typedef int16_t TaskTimeout_t;

///Data type for priority level. Highest value comes first.
typedef uint8_t TaskPriorityLevel_t;



typedef enum TaskSchedule_t
{
	TASK_SCHEDULE_ROUND_ROBIN,
	TASK_SCHEDULE_PRIORITY
}
/**
* \brief The task scheduling algorithm/Schedule type to use
*/
TaskSchedule_t;



typedef enum TaskStatus_t
{
	TASK_NONE = 0,
	TASK_READY = 1,
	TASK_BLOCKED = 2,
	TASK_SLEEP = 3,
	TASK_YIELD = 4,
	TASK_MAIN = 5, //Special reserved status for 'main' tasks
	TASK_SCHEDULED = 6,
	TASK_KILL = 7
	
}
/**
*
* \brief enum for the status of our tasks
*
*/
TaskStatus_t;



typedef union VptrSplit_t
{
	struct
	{
		//Pointer low byte
		uint8_t low;

		//Pointer high byte
		uint8_t high;
			
	} 
	//Structure for the bytes in the union
	bytes;
	
	//The void pointer to split
	void *ptr;
	
} 
/**
 * \brief Structure that splits a void pointer into bytes
 */
VptrSplit_t;



typedef struct TaskContext_t
{
	//The status register value
	TaskRegisterType_t sreg;
	
	//The saved registers
	TaskRegisterType_t registerFile[TASK_REGISTERS];
	
	//The program counter
	VptrSplit_t pc;
	
	//The stack pointer
	VptrSplit_t sp;
	
	
} 
/**
 * \brief Structure for holding program context data
 */
TaskContext_t;



typedef struct TaskControl_t
{
	
	//Context for execution
	TaskContext_t taskExecutionContext;
	
	//The status of our task
	TaskStatus_t taskStatus;

	//Our tasks data
	void* taskData;

	//Void pointer to our tasks function
	void* task_func;
	
	//Current set timeout
	TaskTimeout_t timeout;
	
	//The default timeout value. How long or if any timeout should exist when finishing a count.
	TaskTimeout_t defaultTimeout;
	
	//The ID of this task
	TaskIndiceType_t taskID;
	
	//Allocated space
	void *_taskStack;
	
	//Task priority level, if setting enabled
	TaskPriorityLevel_t priority;
	
	//The next task control
	//struct TaskControl_t *next;
	
} 

/**
* \brief Struct for task controllers
*
*/
TaskControl_t;



//----------------------------------------------------------------------------------------------------


//VARIABLES-------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------




//ASM Macros and NAKED ASM FUNCTIONS------------------------------------------------------------------

///Command to Load a global ptr into Z
#define TaskSchedulerLoadGlobalZPtr(__taskContext) "lds ZL, " #__taskContext " \n\t" "lds ZH, " #__taskContext "+1 \n\t"


///Saves program context position. See naked function for comments.
#define _ASM_SAVE_CONTEXT(_z_load_cmds) \
asm volatile(  \
	"push r30 \n\t" \
	"push r31 \n\t" \
	"in r30, __SREG__ \n\t" \
	"cli \n\t" \
	"push r0 \n\t" \
	"push r30 \n\t" \
	_z_load_cmds \
	"pop r0 \n\t"  \
	"st Z+, r0 \n\t" \
	"pop r0 \n\t" \
	"st z+, r0 \n\t" \
	"st z+, r1 \n\t" \
	"st z+, r2 \n\t" \
	"st z+, r3 \n\t" \
	"st z+, r4 \n\t" \
	"st z+, r5 \n\t" \
	"st z+, r6 \n\t" \
	"st z+, r7 \n\t" \
	"st z+, r8 \n\t" \
	"st z+, r9 \n\t" \
	"st z+, r10 \n\t" \
	"st z+, r11 \n\t" \
	"st z+, r12 \n\t" \
	"st z+, r13 \n\t" \
	"st z+, r14 \n\t" \
	"st z+, r15 \n\t" \
	"st z+, r16 \n\t" \
	"st z+, r17 \n\t" \
	"st z+, r18 \n\t" \
	"st z+, r19 \n\t" \
	"st z+, r20 \n\t" \
	"st z+, r21 \n\t" \
	"st z+, r22 \n\t" \
	"st z+, r23 \n\t" \
	"st z+, r24 \n\t" \
	"st z+, r25 \n\t" \
	"st z+, r26 \n\t" \
	"st z+, r27 \n\t" \
	"st z+, r28 \n\t" \
	"st z+, r29 \n\t" \
	"mov r28, r30 \n\t" \
	"mov r29, r31 \n\t" \
	"pop r31 \n\t" \
	"pop r30 \n\t" \
	"st y+, r30 \n\t" \
	"st y+, r31 \n\t" \
	"pop r30 \n\t" \
	"pop r31 \n\t"  \
	"st y+, r31 \n\t" \
	"st y+, r30 \n\t" \
	"in r26, __SP_L__ \n\t" \
	"in r27, __SP_H__ \n\t" \
	"st y+, r26 \n\t"       \
	"st y, r27  \n\t" \
	"push r31 \n\t" \
	"push r30 \n\t" \
	"mov r30, r28 \n\t"    \
	"mov r31, r29 \n\t"                               \
	"in r28, __SREG__ \n\t" \
	"sbiw r30, CONTEXT_OFFSET_R26 \n\t" \
	"out __SREG__, r28 \n\t"   \
	"ld r26, Z+ \n\t"                                                  \
	"ld r27, Z+ \n\t"                                                  \
	"ld r28, Z+ \n\t"                                                  \
	"ld r29, Z+ \n\t"                   \
	"push r28 \n\t"                                                    \
	"push r29 \n\t"        \
	"mov r28, r30 \n\t"                                                \
	"mov r29, r31 \n\t"                                                \
	"ld r30, Y+ \n\t"                                                  \
	"ld r31, Y  \n\t" \
	"pop r29 \n\t"                                                     \
	"pop r28 \n\t"::)



///Restores program context position. See naked function for comments.
#define _ASM_RESTORE_CONTEXT(_z_load_cmds) \
asm volatile( \
	_z_load_cmds \
	"adiw r30, CONTEXT_OFFSET_SP_H \n\t" \
	"cli \n\t" \
	"ld r0, Z \n\t" \
	"out __SP_H__, r0 \n\t" \
	"ld r0, -Z \n\t" \
	"out __SP_L__, r0 \n\t" \
	"ld r1, -Z \n\t" \
	"ld r0, -Z \n\t" \
	"push r0 \n\t" \
	"push r1 \n\t" \
	"mov r28, r30 \n\t"                                                 \
	"mov r29, r31 \n\t"                                                 \
	"ld r31, -Y \n\t"                                                   \
	"ld r30, -Y \n\t"                                                   \
	"push r31 \n\t"                                                     \
	"push r30 \n\t"                                                     \
	"mov r30, r28 \n\t"                                                 \
	"mov r31, r29 \n\t"                                                 \
	"ld r29, -Z \n\t" \
	"ld r28, -Z \n\t" \
	"ld r27, -Z \n\t" \
	"ld r26, -Z \n\t" \
	"ld r25, -Z \n\t" \
	"ld r24, -Z \n\t" \
	"ld r23, -Z \n\t" \
	"ld r22, -Z \n\t" \
	"ld r21, -Z \n\t" \
	"ld r20, -Z \n\t" \
	"ld r19, -Z \n\t" \
	"ld r18, -Z \n\t" \
	"ld r17, -Z \n\t" \
	"ld r16, -Z \n\t" \
	"ld r15, -Z \n\t" \
	"ld r14, -Z \n\t" \
	"ld r13, -Z \n\t" \
	"ld r12, -Z \n\t" \
	"ld r11, -Z \n\t" \
	"ld r10, -Z \n\t" \
	"ld r9, -Z \n\t" \
	"ld r8, -Z \n\t" \
	"ld r7, -Z \n\t" \
	"ld r6, -Z \n\t" \
	"ld r5, -Z \n\t" \
	"ld r4, -Z \n\t" \
	"ld r3, -Z \n\t" \
	"ld r2, -Z \n\t" \
	"ld r1, -Z \n\t" \
	"ld r0, -Z \n\t" \
	"push r0 \n\t" \
	"ld r0, -Z \n\t" \
	"out __SREG__, r0 \n\t" \
	"pop r0 \n\t" \
	"pop r30 \n\t" \
	"pop r31 \n\t"::)





///Restores a global pointers context. See naked functions for comments.
#define ASM_SAVE_GLOBAL_PTR_CONTEXT(_ptr) \
_ASM_SAVE_CONTEXT( "lds ZL, " #_ptr " \n\t" "lds ZH, " #_ptr "+1 \n\t")



///Restores a global pointers context. See naked functions for comments.
#define ASM_RESTORE_GLOBAL_PTR_CONTEXT(_ptr) \
_ASM_RESTORE_CONTEXT( "lds ZL, " #_ptr " \n\t" "lds ZH, " #_ptr " + 1 \n\t")



/**
 * \brief Saves program context into the passed Context
 * \param taskContext The context structure to save to
 */
__attribute__((naked)) static void SaveContext(volatile TaskContext_t *taskContext)
{
	
	asm volatile
	(
		//Push the Z registers onto the stack
		"push r30 \n\t"
		"push r31 \n\t"
		
		//Save SREG
		"in r30, __SREG__ \n\t"
		
		//"\n" presave_code "\n\t"
		
		//Disable Interrupts
		"cli \n\t"
		
		//Save r0 temp register
		"push r0 \n\t"
		
		//Push SREG value onto the stack
		"push r30 \n\t"
		
		//Move data from the Z address
		"mov r30, r24 \n\t"
		"mov r31, r25 \n\t"
		 
		//Pop from the stack into R0 register
		"pop r0 \n\t" 
		
		/*
		
			Task context formatting:
			
			1st: SREG value
			2nd: Stored register values
			3rd: Program counter
			4th: Stack Pointer
		
		*/
		
		//Save SREG to our context structure 
		"st Z+, r0 \n\t"                      
		
		//Restore initial R0 value by popping the stack into R0.
		"pop r0 \n\t"
		 
		// Save general purpose register file values.
		"st z+, r0 \n\t"
		"st z+, r1 \n\t"
		"st z+, r2 \n\t"
		"st z+, r3 \n\t"
		"st z+, r4 \n\t"
		"st z+, r5 \n\t"
		"st z+, r6 \n\t"
		"st z+, r7 \n\t"
		"st z+, r8 \n\t"
		"st z+, r9 \n\t"
		"st z+, r10 \n\t"
		"st z+, r11 \n\t"
		"st z+, r12 \n\t"
		"st z+, r13 \n\t"
		"st z+, r14 \n\t"
		"st z+, r15 \n\t"
		"st z+, r16 \n\t"
		"st z+, r17 \n\t"
		"st z+, r18 \n\t"
		"st z+, r19 \n\t"
		"st z+, r20 \n\t"
		"st z+, r21 \n\t"
		"st z+, r22 \n\t"
		"st z+, r23 \n\t"
		"st z+, r24 \n\t"
		"st z+, r25 \n\t"
		"st z+, r26 \n\t"
		"st z+, r27 \n\t"
		"st z+, r28 \n\t"
		"st z+, r29 \n\t"
		
		//Move r30 and r31 into r28 and r29 (28 and 29 have already been stored in our struct)
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		
		//Pop from the stack into R31 and R30
		"pop r31 \n\t"
		"pop r30 \n\t"
		
		//Store r30 and r31 at y+
		"st y+, r30 \n\t"
		"st y+, r31 \n\t"
									
		//Pop what should now be the return address 
		"pop r30 \n\t" // high part
		"pop r31 \n\t" // low part 
		
		//and save at Y post increment
		"st y+, r31 \n\t"
		"st y+, r30 \n\t"
		
		//Store our stack pointer into r26 and r27
		"in r26, __SP_L__ \n\t"                                            
		"in r27, __SP_H__ \n\t"
		
		//Save the stack pointer into the structure.                                     
		"st y+, r26 \n\t"                                                  
		"st y, r27  \n\t"
													
		//Push the return address back at the top of the stack.      
		"push r31 \n\t" // low part                                      
		"push r30 \n\t" // high part  
		                                 
		//Context now saved
		 
		//But...registers 26, 27, 28, 29, 30, and 31 are now clobbered.
		
		//To provide generic usage, restore the values even if we don't need to.
		
		//Switch from Y pointer register to Z   
		"mov r30, r28 \n\t"   
		"mov r31, r29 \n\t"                              
		
		//Save our SREG value into R28
		"in r28, __SREG__ \n\t"
		
		//Go to the offset of R26 in our context structure          
		"sbiw r30, CONTEXT_OFFSET_R26 \n\t"
		
		//Restore our SREG value
		"out __SREG__, r28 \n\t"  
		                     
		//Load registers 26-29 from our data structure
		"ld r26, Z+ \n\t"                                                 
		"ld r27, Z+ \n\t"                                                 
		"ld r28, Z+ \n\t"                                                 
		"ld r29, Z+ \n\t"                  
									
		//Push R28, R29 (Y) on the stack to save
		"push r28 \n\t"                                                   
		"push r29 \n\t"       
										
		//Switch to our other index register (z to y) and read r30 and r31
		"mov r28, r30 \n\t"                                               
		"mov r29, r31 \n\t"                                               
		"ld r30, Y+ \n\t"                                                 
		"ld r31, Y  \n\t"                     
									
		//Restore R28, R29 (Y index) from the stack
		"pop r29 \n\t"                                                    
		"pop r28 \n\t"
		 
		:
		: 
		
	);
	
	
	__asm__ __volatile__ ("ret\n");
}



/**
 * \brief Saves program context into passed Context A and restores from passed Context B
 * \param taskContextA The context structure to save to
 * \param taskContextB The context structure to restore from
 */
__attribute__((naked)) static void SwapContext(volatile TaskContext_t *taskContextA, volatile TaskContext_t *taskContextB)
{

	/*
		Save Context
	*/
	asm volatile
	(
		//Push the Z registers onto the stack
		"push r30 \n\t"
		"push r31 \n\t"
		
		//Save SREG
		"in r30, __SREG__ \n\t"
		
		//"\n" presave_code "\n\t"
		
		//Disable Interrupts
		"cli \n\t"
		
		//Save r0 temp register
		"push r0 \n\t"
		
		//Push SREG value onto the stack
		"push r30 \n\t"

		//Move data from the Z address
		 "mov r30, r22 \n\t"
		 "mov r31, r23 \n\t"
		 
		//Pop from the stack into R0 register
		"pop r0 \n\t" 
		
		
		/*
		
		Task context formatting:
		
		1st: SREG value
		2nd: Stored register values
		3rd: Program counter
		4th: Stack Pointer
		
		*/
		
		//Save SREG to our context structure 
		"st Z+, r0 \n\t"                      
		
		//Restore initial R0 value by popping the stack into R0.
		"pop r0 \n\t"
		
		// Save general purpose register file values.
		"st z+, r0 \n\t"
		"st z+, r1 \n\t"
		"st z+, r2 \n\t"
		"st z+, r3 \n\t"
		"st z+, r4 \n\t"
		"st z+, r5 \n\t"
		"st z+, r6 \n\t"
		"st z+, r7 \n\t"
		"st z+, r8 \n\t"
		"st z+, r9 \n\t"
		"st z+, r10 \n\t"
		"st z+, r11 \n\t"
		"st z+, r12 \n\t"
		"st z+, r13 \n\t"
		"st z+, r14 \n\t"
		"st z+, r15 \n\t"
		"st z+, r16 \n\t"
		"st z+, r17 \n\t"
		"st z+, r18 \n\t"
		"st z+, r19 \n\t"
		"st z+, r20 \n\t"
		"st z+, r21 \n\t"
		"st z+, r22 \n\t"
		"st z+, r23 \n\t"
		"st z+, r24 \n\t"
		"st z+, r25 \n\t"
		"st z+, r26 \n\t"
		"st z+, r27 \n\t"
		"st z+, r28 \n\t"
		"st z+, r29 \n\t"
		
		//Move r30 and r31 into r28 and r29 (28 and 29 have already been stored in our struct)
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		
		//Pop from the stack into R31 and R30
		"pop r31 \n\t"
		"pop r30 \n\t"
		
		//Store r30 and r31 at y+
		"st y+, r30 \n\t"
		"st y+, r31 \n\t"

		//Pop what should now be the return address 
		"pop r30 \n\t" // high part
		"pop r31 \n\t" // low part 
		
		//and save at Y post increment
		"st y+, r31 \n\t"
		"st y+, r30 \n\t"
		
		//Store our stack pointer into r26 and r27
		"in r26, __SP_L__ \n\t"
		"in r27, __SP_H__ \n\t"
		
		//Save the stack pointer into the structure.
		"st y+, r26 \n\t"
		"st y, r27  \n\t"

		//Push the return address back at the top of the stack.
		"push r31 \n\t" // low part
		"push r30 \n\t" // high part

		//Context now saved
		
		//But...registers 26, 27, 28, 29, 30, and 31 are now clobbered.
		
		//To provide generic usage, restore the values even if we don't need to.
		
		//Switch from Y pointer register to Z   
		"mov r30, r28 \n\t"
		"mov r31, r29 \n\t"
		
		//Save our SREG value into R28
		"in r28, __SREG__ \n\t"
		
		//Go to the offset of R26 in our context structure          
		"sbiw r30, CONTEXT_OFFSET_R26 \n\t"
		
		//Restore our SREG value
		"out __SREG__, r28 \n\t" 
	
		//Load registers 26-29 from our data structure
		"ld r26, Z+ \n\t"
		"ld r27, Z+ \n\t"
		"ld r28, Z+ \n\t"
		"ld r29, Z+ \n\t"

		//Push R28, R29 (Y) on the stack to save
		"push r28 \n\t"
		"push r29 \n\t"

		//Switch to our other index register (z to y) and read r30 and r31
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		"ld r30, Y+ \n\t"
		"ld r31, Y  \n\t"

		//Restore R28, R29 (Y index) from the stack
		"pop r29 \n\t"
		"pop r28 \n\t"

		:
		:
		
	);
	
	/*
		Restore Context
	*/
	asm volatile
	(
		//Move data from the Z address
		 "mov r30, r24 \n\t"
		 "mov r31, r25 \n\t"
		//_LOAD_ADDRESS_Z                               

		//Go to the end of the context structure and start restoring it from there.
		"adiw r30, CONTEXT_OFFSET_SP_H \n\t"
		
		//Disable interrupts
		"cli \n\t"
		
		//Restore the saved stack pointer.
		"ld r0, Z \n\t"
		"out __SP_H__, r0 \n\t"
		"ld r0, -Z \n\t"
		"out __SP_L__, r0 \n\t"
		
		//Put the saved return address (PC) back on the top of the stack.
		"ld r1, -Z \n\t" //high part 
		"ld r0, -Z \n\t" //low part 
		
		"push r0 \n\t"
		"push r1 \n\t"
		
		//Temporarily switch pointer from Z index to Y index,
		//restore r31, r30 (Z) and put them on top of the stack.     
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		"ld r31, -Y \n\t"
		"ld r30, -Y \n\t"
		"push r31 \n\t"
		"push r30 \n\t"
		
		//Switch back from Y index to Z index. 
		"mov r30, r28 \n\t"
		"mov r31, r29 \n\t"
		
		//Restore general purpose file registers.                   
		"ld r29, -Z \n\t"
		"ld r28, -Z \n\t"
		"ld r27, -Z \n\t"
		"ld r26, -Z \n\t"
		"ld r25, -Z \n\t"
		"ld r24, -Z \n\t"
		"ld r23, -Z \n\t"
		"ld r22, -Z \n\t"
		"ld r21, -Z \n\t"
		"ld r20, -Z \n\t"
		"ld r19, -Z \n\t"
		"ld r18, -Z \n\t"
		"ld r17, -Z \n\t"
		"ld r16, -Z \n\t"
		"ld r15, -Z \n\t"
		"ld r14, -Z \n\t"
		"ld r13, -Z \n\t"
		"ld r12, -Z \n\t"
		"ld r11, -Z \n\t"
		"ld r10, -Z \n\t"
		"ld r9, -Z \n\t"
		"ld r8, -Z \n\t"
		"ld r7, -Z \n\t"
		"ld r6, -Z \n\t"
		"ld r5, -Z \n\t"
		"ld r4, -Z \n\t"
		"ld r3, -Z \n\t"
		"ld r2, -Z \n\t"
		"ld r1, -Z \n\t"
		"ld r0, -Z \n\t"
		
		//Push R0 onto the stack to save
		"push r0 \n\t"
		
		//Restore SREG from our structure
		"ld r0, -Z \n\t"
		"out __SREG__, r0 \n\t"
		
		//Pop R0 back off the stack
		"pop r0 \n\t"
		
		//Restore r31, r30 (Z index) from the stack.
		"pop r30 \n\t"
		"pop r31 \n\t"
		
		
		//Enable Global Interrupts
		"sei \n\t"
		
		:
		:
	);
	
	//Return
	asm volatile("reti \n\t");
	//__asm__ __volatile__ ("ret\n");
}



/**
 * \brief Restores from passed Context
 * \param taskContext The context structure to restore from
 */
__attribute__((naked)) static void RestoreContext(volatile TaskContext_t *taskContext)
{
	
	/*
		Restore Context
	*/
	asm volatile
	(
		//Move data from the Z address
		"mov r30, r24 \n\t"
		"mov r31, r25 \n\t"

		//Go to the end of the context structure and start restoring it from there.
		"adiw r30, CONTEXT_OFFSET_SP_H \n\t"
		
		//Disable interrupts
		"cli \n\t"
		
		//Restore the saved stack pointer.
		"ld r0, Z \n\t"
		"out __SP_H__, r0 \n\t"
		"ld r0, -Z \n\t"
		"out __SP_L__, r0 \n\t"
		
		//Put the saved return address (PC) back on the top of the stack.
		"ld r1, -Z \n\t" //high part 
		"ld r0, -Z \n\t" //low part 
		
		"push r0 \n\t"
		"push r1 \n\t"
		
		//Temporarily switch pointer from Z to Y,                    
		//restore r31, r30 (Z) and put them on top of the stack.     
		"mov r28, r30 \n\t"                                                
		"mov r29, r31 \n\t"                                                
		"ld r31, -Y \n\t"                                                  
		"ld r30, -Y \n\t"                                                  
		"push r31 \n\t"                                                    
		"push r30 \n\t"                                                    
		
		//Switch back from Y to Z.                                   
		"mov r30, r28 \n\t"                                                
		"mov r31, r29 \n\t"                                                
		
		//Restore other general purpose registers.                   
		"ld r29, -Z \n\t"
		"ld r28, -Z \n\t"
		"ld r27, -Z \n\t"
		"ld r26, -Z \n\t"
		"ld r25, -Z \n\t"
		"ld r24, -Z \n\t"
		"ld r23, -Z \n\t"
		"ld r22, -Z \n\t"
		"ld r21, -Z \n\t"
		"ld r20, -Z \n\t"
		"ld r19, -Z \n\t"
		"ld r18, -Z \n\t"
		"ld r17, -Z \n\t"
		"ld r16, -Z \n\t"
		"ld r15, -Z \n\t"
		"ld r14, -Z \n\t"
		"ld r13, -Z \n\t"
		"ld r12, -Z \n\t"
		"ld r11, -Z \n\t"
		"ld r10, -Z \n\t"
		"ld r9, -Z \n\t"
		"ld r8, -Z \n\t"
		"ld r7, -Z \n\t"
		"ld r6, -Z \n\t"
		"ld r5, -Z \n\t"
		"ld r4, -Z \n\t"
		"ld r3, -Z \n\t"
		"ld r2, -Z \n\t"
		"ld r1, -Z \n\t"
		"ld r0, -Z \n\t"
		
		//Restore SREG
		"push r0 \n\t"
		"ld r0, -Z \n\t"
		"out __SREG__, r0 \n\t"
		"pop r0 \n\t"
		
		//Restore r31, r30 (Z) from the stack.
		"pop r30 \n\t"
		"pop r31 \n\t"
		

		//Enable Global Interrupts
		"sei \n\t"
		:
		:
	);
	
	//Return
	__asm__ __volatile__ ("ret\n");
}



//----------------------------------------------------------------------------------------------------






//FUNCTIONS-------------------------------------------------------------------------------------------



extern __attribute__ ((weak)) void _TaskSwitch(void);

extern void SetTaskDefaultTimeout(TaskIndiceType_t id, TaskTimeout_t timeout);
extern void SetTaskSchedule(TaskSchedule_t schedule);
extern void SetTaskPriority(TaskIndiceType_t id, TaskPriorityLevel_t priority);
extern const bool AreTaskRunning();
extern const TaskIndiceType_t GetCurrentTask();
extern const TaskIndiceType_t _GetTaskID(TaskIndiceType_t index);
extern TaskIndiceType_t _GetTaskIndex(TaskIndiceType_t id);
extern TaskStatus_t GetTaskStatus(TaskIndiceType_t id);
extern void SetTaskStatus(TaskIndiceType_t id, TaskStatus_t status);


extern void AttachIDTask(void (*func)(TaskIndiceType_t));
extern void AttachTask(void (*func)(void ));
extern void AttachTaskAt(void (*func)(void), TaskIndiceType_t index);
extern void AttachTaskPointer(void *taskPtr);
extern void AttachTaskPointerAt(void *taskPtr, TaskIndiceType_t index);

extern int8_t _KillTaskImmediate(TaskIndiceType_t index);
extern int8_t KillTask(TaskIndiceType_t index);
extern int8_t KillAllTasks();
extern int8_t _KillAllTasksImmediate();
extern TaskControl_t* _GetTaskControl(TaskIndiceType_t index);
extern void DispatchTasks();
extern void StartTasks(void (*mainfunc)(void));
extern void _EmptyTask(void);


extern void BROKEN_TaskBlockOthers(TaskIndiceType_t tid);
extern void BROKEN_TaskFreeOthers(TaskIndiceType_t tid);


extern void TaskSleep(TaskIndiceType_t taskIndex, TaskTimeout_t counts);
extern void TaskYield(TaskTimeout_t counts);
extern void TaskSetYield(TaskIndiceType_t taskIndex, TaskTimeout_t counts);


extern uint8_t TaskYieldRequestDataCopy(TaskIndiceType_t id, void *memDestinationAddress, void *memSourceAddress, uint8_t bytes);
extern uint8_t TaskRequestDataCopy(void *memDestinationAddress, void *memSourceAddress, uint8_t bytes);
extern uint8_t TaskYieldWriteData(TaskIndiceType_t id, void *memDestinationAddress, void *data, uint8_t bytes);
extern uint8_t TaskRequestDataWrite(void *memDestinationAddress, void *data, uint8_t bytes);


///Helper for writing to a port, or a register, or whatever, when passing constant values. Ex. TaskWriteData(PORTB, 0x0F, writeStatus); 
#define TaskWriteData(_memDestination, _dataToWrite, _writeStatusOut)  do { typeof(_dataToWrite) _d = _dataToWrite; _writeStatusOut = TaskRequestDataWrite((void *)&_memDestination, &_d, sizeof(_d));  } while(0)

///Helper for writing to a port, or a register, or whatever, when passing constant values. Ex. TaskWaitForDataWrite(PORTB, 0x0F); 
#define TaskWaitForDataWrite(_memDestination, _dataToWrite)  do { typeof(_dataToWrite) _d = _dataToWrite; while(!TaskRequestDataWrite((void *)&_memDestination, &_d, sizeof(_d)));  } while(0)

///Helper for writing to a port, or a register, or whatever, when passing constant values. Ex. TaskYieldForDataWrite(taskID, PORTB, 0x0F); 
#define TaskYieldForDataWrite(_taskID, _memDestination, _dataToWrite)  do { typeof(_dataToWrite) _d = _dataToWrite; TaskYieldWriteData(_taskID, (void *)&_memDestination, &_d, sizeof(_d) );  } while(0)


//----------------------------------------------------------------------------------------------------




/**
* \brief Schedules anything, but what should be a function, at the index passed. Can be dangerous if not used properly.
* \param taskPtr the address of what should be a function
* \param Returns the id for the scheduled task
*/
static TaskIndiceType_t ScheduleTaskPointer(void *taskPtr)
{
	TaskIndiceType_t nid = -1;
	volatile TaskStatus_t taskStatus = TASK_BLOCKED;
	
	for(nid = 0; nid < MAX_TASKS; nid++)
	{
		taskStatus = GetTaskStatus(nid);
		
		if(taskStatus == TASK_NONE)
		{
			AttachTaskPointerAt(taskPtr, nid);
			SetTaskStatus(nid, TASK_SCHEDULED);
			break;
		}
	}
	
	return nid;
}



/**
* \brief Adds and schedules a task to the available tasks for it to be ran when able
* \param func The function for running the task
* \param Returns the id for the scheduled task
*/
static TaskIndiceType_t ScheduleTask(void (*func)(void))
{
	TaskIndiceType_t nid = -1;
	volatile TaskStatus_t taskStatus = TASK_BLOCKED;
	
	for(nid = 0; nid < MAX_TASKS; nid++)
	{
		taskStatus = GetTaskStatus(nid);
		
		if(taskStatus == TASK_NONE)
		{
			AttachTaskAt(func, nid);
			SetTaskStatus(nid, TASK_SCHEDULED);
			break;
		}
	}
	
	return nid;
}





#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* __PREEMPTIVETASKSCHEDULER_H__ */
