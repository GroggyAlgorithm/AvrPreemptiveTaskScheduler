/**
 * \file PreemptiveTaskSchedulerTypes.h
 * \author: Tim Robbins
 * \brief Data types file for preemptive task scheduling and concurrent functionality. \n
 */ 
#ifndef __PREEMPTIVETASKSCHEDULERTYPES_H___
#define __PREEMPTIVETASKSCHEDULERTYPES_H___	1



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


#include "PreemptiveTaskSchedulerConfig.h"

#include <stdbool.h>
#include <stdint.h>



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
typedef int16_t TaskPriorityLevel_t;


//TASK_SCHEDULE_LRT, //Lowest remaining time, didn't make work well ): 

typedef enum TaskSchedule_t
{
	TASK_SCHEDULE_ROUND_ROBIN = 0,

	///Runs based on the next highest priority out of the priorities that have not been run yet
	TASK_SCHEDULE_PRIORITY = 1,

	///Strictly selects next based on priorities that must be changed elsewhere
	TASK_SCHEDULE_PRIORITY_STRICT = 2, 
	
	///Prioritizes tasks marked with the status 'main' and runs them ever other interrupt
	TASK_SCHEDULE_PRIORITY_MAIN = 3,
	
	///physically reorders the task collection based on priorities
	TASK_SCHEDULE_PRIORITY_REORDER = 4,
	
	///Runs based on the next highest priority out of the priorities that have not been run yet but only if the task is set as READY or is tagged as the MAIN task
	TASK_SCHEDULE_PRIORITY_AND_READY = 5
	
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
		//Pointer low
		uint8_t low;

		//Pointer high
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
	
	//The ID of this task
	TaskIndiceType_t taskID;
	
	//Allocated space
	void *_taskStack;
	
	//The default timeout value. How long or if any timeout should exist when finishing a count.
	TaskTimeout_t defaultTimeout;
	
	//Task priority level, if setting enabled
	TaskPriorityLevel_t priority;
	
	//Saved priority level
	TaskPriorityLevel_t cachedPriority;
}

/**
* \brief Struct for task controllers
*
*/
TaskControl_t;



typedef struct TaskControlNode_t
{
	
	//The current Task Control value
	struct TaskControl_t *control;
	
	//The next task control node
	struct TaskControlNode_t *next;
	
	
} 
/**
* \brief Struct for control node data structures, such as queues and stacks
*
*/
TaskControlNode_t;



//----------------------------------------------------------------------------------------------------



#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif /* __PREEMPTIVETASKSCHEDULERTYPES_H___ */