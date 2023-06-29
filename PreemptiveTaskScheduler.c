/**
 * \file PreemptiveTaskScheduler.c
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and concurrent functionality.
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "PreemptiveTaskScheduler.h"

///An array block of our task control structures
static TaskControl_t m_TaskControl[MAX_TASKS+1];

///The index for our Task control block structure
static TaskIndiceType_t m_TaskBlockIndex = 0;

///Count of the total items we've placed into the task control block
static TaskIndiceType_t m_TaskBlockCount = 0;

///The current task context
volatile TaskControl_t *m_CurrentTask;



/**
* \brief Returns the current task ID
*/
const inline TaskIndiceType_t GetCurrentTask()
{
	//Return the id to our pointers current task id
	return m_CurrentTask->taskID;
}



/**
* \brief Gets the ID at the passed task index
* \param index The index to fetch the ID at
*/
const TaskIndiceType_t _GetTaskID(TaskIndiceType_t index)
{
	//If our index is within range, return our id, else return out of range
	return (index >= 0 && index <= MAX_TASKS) ? m_TaskControl[index].taskID : -1;	
}



/**
* \brief Gets the FIRST index for the passed task ID
* \param index The index to fetch the ID at
*/
TaskIndiceType_t _GetTaskIndex(TaskIndiceType_t id)
{
	//Create an index value, initialized to out of range
	TaskIndiceType_t index = -1;
	
	//While we're within range of our tasks...
	for(TaskIndiceType_t i = 0; i <= MAX_TASKS; i++)
	{
		//If we've found our ID...
		if(m_TaskControl[i].taskID == id)
		{
			//Set our return value
			index = i;
			
			//Break
			break;
		}
	}
	
	return index;
	
}



/**
* \brief Returns the status of the specified task
* \param index The task id to check at
* \ret The status of the task
*/
inline TaskStatus_t GetTaskStatus(TaskIndiceType_t id)
{
	//If our ID is within range, return our status, else return none
	return (id >= 0 && id <= MAX_TASKS) ? m_TaskControl[id].taskStatus : TASK_NONE;	
}




/**
* \brief Sets the status of the specified task
* \param index The task id to set at
* \param status The status of the task
*/
inline void SetTaskStatus(TaskIndiceType_t id, TaskStatus_t status)
{
	//If our ID is within range...
	if(id >= 0 && id < MAX_TASKS)
	{
		//Set our status
		m_TaskControl[id].taskStatus = status;	
	}
	 
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
		if((uint16_t)_TASK_STACK_START_ADDRESS(m_TaskBlockIndex) < RAMSTART)
		{
			//Out of memory ):
			return;	
		}
		
		//Set the task ID
		m_TaskControl[m_TaskBlockIndex].taskID = m_TaskBlockIndex;
		
		//Set the memory location for our task stack
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
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[m_TaskBlockIndex]._taskStack);
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.pc.ptr = (void *)func;
		
#warning Hey, this isnt working anymore? what the heck! 
		//Initialize register for argument 1
		addr = (TaskIndiceType_t)m_TaskControl[m_TaskBlockIndex].taskID;
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
		//
		
		
		//Increment our index
		m_TaskBlockIndex++;
		
		//Set our count to be equal to the index
		m_TaskBlockCount = m_TaskBlockIndex;
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

		//Check for memory allowances
		if((uint16_t)_TASK_STACK_START_ADDRESS(m_TaskBlockIndex) < RAMSTART)
		{
			//Out of memory ):
			return;	
		}
		
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
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[m_TaskBlockIndex]._taskStack);
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.pc.ptr = (void *)func;

		//Increment our index
		m_TaskBlockIndex++;
		
		//Set our count to be equal to the index
		m_TaskBlockCount = m_TaskBlockIndex;
		
		
	}
	
	
	
}



/**
* \brief Attaches a task to the available tasks
* \param func The function for running the task
* \param index The position to attach at
*/
void AttachTaskAt(void (*func)(void), TaskIndiceType_t index)
{
	//If we have the ability to add a new block...
	if(index < MAX_TASKS && index >= 0)
	{
		
		//Check for memory allowances
		if((uint16_t)_TASK_STACK_START_ADDRESS(index) < RAMSTART)
		{
			//Out of memory ):
			return;	
		}
		
		//Set the memory location for our task stack
		m_TaskControl[index]._taskStack = _TASK_STACK_START_ADDRESS(index);
		
		//Set the task ID
		m_TaskControl[index].taskID = index;
		
		//Set the function
		m_TaskControl[index].task_func = (void *)func;
		
		//Default to blocked until tasks are dispatched
		m_TaskControl[index].taskStatus = TASK_BLOCKED;
		
		//Set our default timeout
		m_TaskControl[index].timeout = TASK_DEFAULT_TIMEOUT;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[index].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[index]._taskStack);
		m_TaskControl[index].taskExecutionContext.pc.ptr = (void *)func;

		//Increment our index
		index++;
		
		//Set our count to be equal to the index
		m_TaskBlockCount = index;
		
		
	}
	
	
	
}



/**
* \brief Immediately kills any task that has the passed id
* \param index The index to find and kill
* \ret 0 if not killed, the 1 if killed
*/
int8_t _KillTaskImmediate(TaskIndiceType_t index)
{
	//If our ID is out of range...
	if(index < 0 || index > MAX_TASKS)
	{
		//Return 0
		return 0;
	}
	
	//Set initially to blocked so we don't call the task
	m_TaskControl[index].taskStatus = TASK_BLOCKED;
	
	//Set the tasks stack address to 0
	m_TaskControl[index]._taskStack = 0;
		
	//Loop through the tasks registers and...
	for(TaskIndiceType_t i = 0; i < TASK_REGISTERS; i++)
	{
		//Clear
		m_TaskControl[index].taskExecutionContext.registerFile[i] = 0;
	}
	
	//Clear data
	m_TaskControl[index].taskExecutionContext.pc.ptr = 0;
	m_TaskControl[index].taskExecutionContext.sp.ptr = 0;
	m_TaskControl[index].taskExecutionContext.sreg = 0;
	m_TaskControl[index].taskData = 0;
	m_TaskControl[index].task_func = 0;
	
	//Reset our timeout to its default
	m_TaskControl[index].timeout = TASK_DEFAULT_TIMEOUT;
	
	//Set the id to out of range
	m_TaskControl[index].taskID = -1;
	
	//Set the status as available
	m_TaskControl[index].taskStatus = TASK_NONE;
	
	//Decrease our count
	m_TaskBlockCount--;
	
	//Return successful
	return 1;
}



/**
* \brief Schedules a task to be killed
* \param index The index of the tasks to kill
* \ret 0 if not killed, 1 if killed
*/
int8_t KillTask(TaskIndiceType_t index)
{
	//If our index is out of range...
	if(index < 0 || index > MAX_TASKS)
	{
		//Return 0
		return 0;
	}
	
	//Set the tasks status to kill
	m_TaskControl[index].taskStatus = TASK_KILL;
	
	//Wait to be killed
	while(m_TaskControl[index].taskStatus == TASK_KILL);
	
	//Return 1
	return 1;
}



/**
* \brief Returns the task control at the passed ID. Not super safe
* \param index the index to get the task at
*/
TaskControl_t* _GetTaskControl(TaskIndiceType_t index)
{
	//If our index is within range...
	if(index < MAX_TASKS && index >= 0 )
	{
		//Return the address for our task at index
		return &m_TaskControl[index];
	}
	//else...
	else
	{
		//return 0
		return 0;
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
		__asm__ __volatile__("cli \n\t":::"memory");
		
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
				m_TaskControl[i].taskStatus = TASK_NONE;

			}
		}

		//Make sure we attach our ending task
		//Make sure last task is set, is reserved. Set its ID as well_
		m_TaskControl[MAX_TASKS].taskStatus = TASK_MAIN;
		m_TaskControl[MAX_TASKS].taskID = MAX_TASKS;
		m_TaskControl[MAX_TASKS]._taskStack = _TASK_STACK_START_ADDRESS(MAX_TASKS);
		
		//Set the function
		m_TaskControl[MAX_TASKS].task_func = (void *)_EmptyTask;
		
		//Set our default timeout
		m_TaskControl[MAX_TASKS].timeout = TASK_DEFAULT_TIMEOUT;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[MAX_TASKS].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[MAX_TASKS]._taskStack + sizeof(TASK_STACK_SIZE) - 1);
		m_TaskControl[MAX_TASKS].taskExecutionContext.pc.ptr = (void *)_EmptyTask;
		
		//Set our current task to the last possible task, this way when entering for the first time we will loop to the first
		m_CurrentTask = &m_TaskControl[MAX_TASKS];
		
		//Launch the ISR
		_SCHEDULER_LAUNCH_ISR();
		
		//Re enable interrupts
		__asm__ __volatile__("sei \n\t":::"memory");
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
		if(i != tid && m_TaskControl[i].taskStatus != TASK_NONE && m_TaskControl[i].taskStatus != TASK_MAIN)
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
	//If our index is out of range...
	if (taskIndex < 0 || taskIndex > MAX_TASKS)
	{
		//return
		return;
	}
	
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Save our timeout counts
	m_TaskControl[taskIndex].timeout = counts;
	
	//Re-enable interrupts
	asm volatile("sei":::"memory");
	
	//While timed out, count down
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
	volatile TaskIndiceType_t taskIndex = _GetTaskIndex(m_CurrentTask->taskID);
	
	//If our task is out of range...
	if (taskIndex < 0 || taskIndex > MAX_TASKS)
	{
		//Re-enable interrupts
		asm volatile("sei":::"memory");
		
		//return
		return;
	}
	
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
	//If our task is out of range...
	if (taskIndex < 0 || taskIndex > MAX_TASKS)
	{
		//return
		return;
	}
	
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
	
	//Get our current task block index
	m_TaskBlockIndex = _GetTaskIndex(m_CurrentTask->taskID);
	
	//If our task index is out of range somehow...
	if(m_TaskBlockIndex < 0 || m_TaskBlockIndex > MAX_TASKS)
	{
		//Return
		return;
	}
	
	//If our task is scheduled to be killed...
	if(m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_KILL)
	{
		m_TaskBlockCount--;
		
		m_TaskControl[m_TaskBlockIndex]._taskStack = 0;
		
		for(TaskIndiceType_t i = 0; i < TASK_REGISTERS; i++)
		{
			m_TaskControl[m_TaskBlockIndex].taskExecutionContext.registerFile[i] = 0;
		}
	
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.pc.ptr = 0;
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sp.ptr = 0;
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sreg = 0;
		m_TaskControl[m_TaskBlockIndex].taskData = 0;
		m_TaskControl[m_TaskBlockIndex].task_func = 0;
		m_TaskControl[m_TaskBlockIndex].timeout = TASK_DEFAULT_TIMEOUT;
		m_TaskControl[m_TaskBlockIndex].taskID = -1;
		m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_NONE;
		
	}
	//else If our new process is not set to blocked...
	else if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_BLOCKED && m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_NONE)
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
	
	//Safety counter
	volatile int8_t safety = 100;
	
	do
	{
		//Increment our block index
		m_TaskBlockIndex++;
		
		//Range check our block index
		if(m_TaskBlockIndex > MAX_TASKS)
		{
			m_TaskBlockIndex = 0;
		}
		
		//If our safety is bad...
		if(--safety <= 0)
		{
			//break
			break;
		}
		
	}while(m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_BLOCKED || m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_NONE || m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_KILL);
	


	//If our new process is not set to a blocked status...
	if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_BLOCKED && m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_NONE && m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_KILL)
	{
		//if our task is set to be scheduled...
		if(m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_SCHEDULED)
		{
			//Set as ready
			m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_READY;
			
			//Go to our empty task
			m_TaskBlockIndex = MAX_TASKS;
			m_CurrentTask = &m_TaskControl[MAX_TASKS];
		}
		else
		{
			//Set our current task context to the new execution context
			m_CurrentTask =  &m_TaskControl[m_TaskBlockIndex];
		}
	}
	//else...
	else
	{
		//Go to our empty task
		m_TaskBlockIndex = MAX_TASKS;
		m_CurrentTask = &m_TaskControl[MAX_TASKS];
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
	ASM_SAVE_GLOBAL_PTR_CONTEXT(m_CurrentTask);
	
	//Handle task switching
	_TaskSwitch();
	
	//Restore our next context
	ASM_RESTORE_GLOBAL_PTR_CONTEXT(m_CurrentTask);
	
	//Make sure isr is reset before exiting...
	_SCHEDULER_LOAD_ISR_REG();

	//Make sure interrupts are re-enabled
	__asm__ __volatile__("sei \n\t":::"memory");
	__asm__ __volatile__("reti \n"::);
}


