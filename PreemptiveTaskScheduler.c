/**
 * \file PreemptiveTaskScheduler.c
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and async functionality.
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "PreemptiveTaskScheduler.h"

///An array block of our task control structures
volatile TaskControl_t m_TaskControl[MAX_TASKS+1];

///The index for our Task control block structure
volatile TaskIndiceType_t m_TaskBlockIndex = 0;

///Count of the total items we've placed into the task control block
volatile TaskIndiceType_t m_TaskBlockCount = 0;

///The current task context
volatile TaskContext_t *m_CurrentTaskContext;

/**
* \brief Returns the current task ID
*/
const inline TaskIndiceType_t GetCurrentTask()
{
	return m_TaskControl[m_TaskBlockIndex].taskID;
}

/**
* \brief Returns the status of the specified task
* \param index The task id to check at
* \ret The status of the task
*/
inline TaskStatus_t GetTaskStatus(TaskIndiceType_t id)
{
	return (id >= 0 && id <= MAX_TASKS) ? m_TaskControl[id].taskStatus : TASK_NONE;	
}



/**
* \brief Attaches a task that specifies a required ID
*
*/
void AttachIDTask(void (*func)(TaskIndiceType_t))
{
	//If we have the ability to add a new block...
	if(m_TaskBlockIndex < MAX_TASKS)
	{
		//Check for memory allowances
		if(_TASK_STACK_START_ADDRESS(m_TaskBlockIndex) < RAMSTART)
		{
			//Out of memory ):
			return;	
		}
		
		//Set the task ID
		m_TaskControl[m_TaskBlockIndex].taskID = m_TaskBlockIndex;
		
		//Set the memory location for our task stack, was previously (void *)calloc(TASK_STACK_SIZE, sizeof(uint8_t));
		m_TaskControl[m_TaskBlockIndex]._taskStack = _TASK_STACK_START_ADDRESS(m_TaskBlockIndex);
		
		//Set the function
		m_TaskControl[m_TaskBlockIndex].task_func = (void *)func;
		
		//Default to blocked until tasks are dispatched
		m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_BLOCKED;
		
		//Set our default timeout
		m_TaskControl[m_TaskBlockIndex].timeout = TASK_DEFAULT_TIMEOUT;
		
		/*
			Function pointer args
		*/
		
		uint16_t addr;
		uint8_t *p = (uint8_t *)&addr;
		
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[m_TaskBlockIndex]._taskStack + sizeof(TASK_STACK_SIZE) - 1);
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.pc.ptr = (void *)func;
		
#warning Hey, this isn't working anymore? what the heck! 
		//Initialize register for argument 1
		addr = (uint16_t)m_TaskControl[m_TaskBlockIndex].taskID;
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[24] = p[0];
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[25] = p[1];


		////Initialize register for argument 2
		//addr = (uint16_t)&m_TaskControl[m_TaskBlockIndex].taskID;
		//m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[22] = p[0];
		//m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[23] = p[1];
		//
		////Initialize register for argument 3
		//addr = (uint16_t)(&m_TaskControl[m_TaskBlockIndex]);
		//m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[20] = p[0];
		//m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[21] = p[1];
		
		
		
		//Increment our index
		m_TaskBlockIndex++;
		
		//Set our count to be equal to the index plus 1
		m_TaskBlockCount = m_TaskBlockIndex+1;
	}
}



/**
* \brief Attaches a task to the available tasks
* \param func The function for running the task
*/
void AttachTask(void (*func)(void))
{
	//If we have the ability to add a new block...
	if(m_TaskBlockIndex < MAX_TASKS)
	{
		//Set the memory location for our task stack
		m_TaskControl[m_TaskBlockIndex]._taskStack = _TASK_STACK_START_ADDRESS(m_TaskBlockIndex);
		
		//Set the task ID
		m_TaskControl[m_TaskBlockIndex].taskID = m_TaskBlockIndex;
		
		//Set the function
		m_TaskControl[m_TaskBlockIndex].task_func = (void *)func;
		
		//Default to blocked until tasks are dispatched
		m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_BLOCKED;
		
		//Set our default timeout
		m_TaskControl[m_TaskBlockIndex].timeout = TASK_DEFAULT_TIMEOUT;
		
		uint16_t addr;
		uint8_t *p = (uint8_t *)&addr;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[m_TaskBlockIndex]._taskStack + sizeof(TASK_STACK_SIZE) - 1);
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.pc.ptr = (void *)func;
		
		//Increment our index
		m_TaskBlockIndex++;
		
		//Set our count to be equal to the index plus 1
		m_TaskBlockCount = m_TaskBlockIndex+1;
	}
	
	
	
}



/**
* \brief Sets all tasks to run and starts the schedulers interrupt service
*
*/
void DispatchTasks()
{
	//If we have tasks to launch...
	if(m_TaskBlockCount > 0)
	{
		//Disable Global interrupts
		asm volatile("cli":::"memory");
		
		//Set our task index to the start
		m_TaskBlockIndex = 0;
	
		//For each task...
		for(TaskIndiceType_t i = 0; i < MAX_TASKS; i++)
		{
			//If we're within range for our count to set to ready...
			if(i < m_TaskBlockCount)
			{
				//Set to ready
				m_TaskControl[i].taskStatus = TASK_READY;
			}
			//else...
			else
			{
				//Set to blocked
				m_TaskControl[i].taskStatus = TASK_BLOCKED;
			}
		}

		//Make sure we attach our ending task
		//Make sure last task is set, is reserved. Set its ID as well_
		m_TaskControl[MAX_TASKS].taskStatus = TASK_MAIN;
		m_TaskControl[MAX_TASKS].taskID = MAX_TASKS;
		
		//Set our current task to the last possible task, this way when entering for the first time we will loop to the first
		m_CurrentTaskContext = &m_TaskControl[MAX_TASKS].taskExecutionContext;
		
		//Launch the ISR
		_SCHEDULER_LAUNCH_ISR();
		
		//Enable Global interrupts
		asm volatile("sei":::"memory");
	}

}



/**
* \brief empty task
*
*/
void _EmptyTask(void)
{
	while (1)
	{
		
	}
}



/**
* \brief Blocks all other tasks
*
*/
void TaskBlockOthers(TaskIndiceType_t tid)
{
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Loop through all tasks and...
	for(TaskIndiceType_t i = 0; i < MAX_TASKS; i++)
	{
		//If it is not this task...
		if(i != tid)
		{
			//Set to blocked
			m_TaskControl[i].taskStatus = TASK_BLOCKED;		
		}
	}

	//Re-enable interrupts
	asm volatile("sei":::"memory");
	
	
}



/**
* \brief Frees all other tasks
*
*/
void TaskFreeOthers(TaskIndiceType_t tid)
{
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Loop through all tasks and...
	for(TaskIndiceType_t i = 0; i < MAX_TASKS; i++)
	{
		//If it is not this task and the task is blocked...
		if(i != tid && m_TaskControl[i].taskStatus == TASK_BLOCKED)
		{
			//Set to ready
			m_TaskControl[i].taskStatus = TASK_READY;	
		}
	}

	//Re-enable interrupts
	asm volatile("sei":::"memory");
}



/**
* \brief Sets this task to sleep for the specified amount of counts, counting down here locally
*
*/
void TaskSleep(TaskIndiceType_t taskIndex, TaskTimeout_t counts)
{
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Save our timeout counts
	m_TaskControl[taskIndex].timeout = counts;
	
	//Re-enable interrupts
	asm volatile("sei":::"memory");
	
	while(m_TaskControl[taskIndex].timeout > 0)
	{
		m_TaskControl[taskIndex].taskStatus = TASK_SLEEP;
		m_TaskControl[taskIndex].timeout -= 1;
	}
	
}



/**
* \brief Sets this task to yield for the specified amount of counts, counting down in the scheduler interrupt
*
*/
void TaskYield(TaskTimeout_t counts)
{
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Save our current index
	volatile TaskIndiceType_t taskIndex = m_TaskBlockIndex;
	
	//Set our status
	m_TaskControl[taskIndex].taskStatus = TASK_YIELD;
	
	//Set our sleep count
	m_TaskControl[taskIndex].timeout = counts;
	
	//Re-enable interrupts
	asm volatile("sei":::"memory");
	
	//Wait while yielded
	while(m_TaskControl[taskIndex].taskStatus == TASK_YIELD);
}



/**
* \brief Sets this task to yield for the specified amount of counts, counting down in the scheduler interrupt
* \param taskIndex The index of the task to sleep
*/
void TaskSetYield(TaskIndiceType_t taskIndex, TaskTimeout_t counts)
{
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Set our status
	m_TaskControl[taskIndex].taskStatus = TASK_YIELD;
	
	//Set our sleep count
	m_TaskControl[taskIndex].timeout = counts;
	
	//Re-enable interrupts
	asm volatile("sei":::"memory");
	
	//Wait while yielded
	while(m_TaskControl[taskIndex].taskStatus == TASK_YIELD);
}



/**
* \brief Handles task switching
*
*/
__attribute__ ((weak)) void _TaskSwitch(void) 
{
	//If our new process is not set to blocked...
	if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_BLOCKED)
	{
		//If our task is set to YIELD...
		if(m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_YIELD)
		{
			//If our task counts are greater than 0...
			if(m_TaskControl[m_TaskBlockIndex].timeout > 0)
			{
				//Decrement our count
				m_TaskControl[m_TaskBlockIndex].timeout--;
			}
			//else...
			else
			{
				//Set task back to ready
				m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_READY;
#if TASK_DEFAULT_TIMEOUT != 0
				//Reset to our default timeout
				m_TaskControl[m_TaskBlockIndex].timeout = TASK_DEFAULT_TIMEOUT;
#endif
			}
		}
	}
	//Increment our block index
	m_TaskBlockIndex++;

	
	//Range check our block index
	if(m_TaskBlockIndex >= m_TaskBlockCount || m_TaskBlockIndex >= MAX_TASKS)
	{
		m_TaskBlockIndex = 0;
	}
	
	//If our new process is not set to blocked...
	if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_BLOCKED)
	{
		//Set our current task context to the new execution context
		m_CurrentTaskContext = &m_TaskControl[m_TaskBlockIndex].taskExecutionContext;	
	}
	
	
}



/**
* \brief Interrupt service routine for our task scheduler
*
*/
ISR(SCHEDULER_INT_VECTOR, ISR_NAKED)
{
	//Make sure interrupts are disabled
	__asm__ __volatile__("cli \n\t":::"memory");
	
	//Save our tasks context
	ASM_SAVE_GLOBAL_PTR_CONTEXT(m_CurrentTaskContext);
	
	//Handle task switching
	_TaskSwitch();
	
	//Restore our next context
	ASM_RESTORE_GLOBAL_PTR_CONTEXT(m_CurrentTaskContext);
	
	//Make sure isr is reset before exiting...
	_SCHEDULER_LOAD_ISR_REG();

	//Make sure interrupts are re-enabled
	__asm__ __volatile__("sei \n\t":::"memory");
	__asm__ __volatile__("reti \n"::);
}


