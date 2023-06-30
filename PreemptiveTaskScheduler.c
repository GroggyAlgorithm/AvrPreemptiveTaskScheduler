/**
 * \file PreemptiveTaskScheduler.c
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and concurrent functionality.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "PreemptiveTaskScheduler.h"

///An array block of our task control structures
static TaskControl_t m_TaskControl[MAX_TASKS+1];

///The index for our Task control block structure
static TaskIndiceType_t m_TaskBlockIndex = 0;

///Count of the total items we've placed into the task control block
static TaskIndiceType_t m_TaskBlockCount = 0;

///The current task context
volatile TaskControl_t *m_CurrentTask;

///Semaphore value for accessing memory, registers, ex. adc, etc.
static SemaphoreValueType_t m_semMemoryAccessor;

///If the tasks have started running
static bool m_blnTasksRunning = false;


/**
* \brief Returns the state of if the tasks are set as running
* \ret bool true if running, false if not running
*/
const inline bool AreTaskRunning()
{
	return m_blnTasksRunning;
}



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
	if(m_TaskBlockIndex < MAX_TASKS && m_TaskBlockIndex >= 0)
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
		
		//Default to scheduled
		m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_SCHEDULED;
		
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
* \brief Safely attaches a task to the available tasks
* \param func The function for running the task
*/
void AttachTask(void (*func)(void))
{
	//If our tasks have started running...
	if(m_blnTasksRunning == true)
	{
		//Schedule the task instead
		ScheduleTask(func);	
	}
	//else, If we have the ability to add a new block...
	else if(m_TaskBlockIndex < MAX_TASKS && m_TaskBlockIndex >= 0)
	{

		//Check for memory allowances
		if((uint16_t)_TASK_STACK_START_ADDRESS(m_TaskBlockIndex) < RAMSTART)
		{
			//Out of memory ):
			return;	
		}
		
		//Check if the current task is open to attaching
		if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_NONE)
		{
			
			//Not a good spot to attach ):
			return;
			
		}
		
		//Set the memory location for our task stack
		m_TaskControl[m_TaskBlockIndex]._taskStack = _TASK_STACK_START_ADDRESS(m_TaskBlockIndex);
		
		//Set the task ID
		m_TaskControl[m_TaskBlockIndex].taskID = m_TaskBlockIndex;
		
		//Set the function
		m_TaskControl[m_TaskBlockIndex].task_func = (void *)func;
		
		//Default to scheduled
		m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_SCHEDULED;
		
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
		
		//Default to scheduled
		m_TaskControl[index].taskStatus = TASK_SCHEDULED;
		
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
* \brief Some what safely attaches anything, but what should be a function. Can be dangerous if not used properly.
* \param taskPtr the address of what should be a function
*/
void AttachTaskPointer(void *taskPtr)
{
	
	//If our tasks have started running...
	if(m_blnTasksRunning == true)
	{
		//Schedule the task instead
		ScheduleTaskPointer(taskPtr);	
	}
	//else, If we have the ability to add a new block...
	else if(m_TaskBlockIndex <= MAX_TASKS && m_TaskBlockIndex >= 0)
	{
		
		//Check for memory allowances
		if((uint16_t)_TASK_STACK_START_ADDRESS(m_TaskBlockIndex) < RAMSTART)
		{
			//Out of memory ):
			return;	
		}
		
		//Check if the current task is open to attaching
		if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_NONE)
		{
			//Not a good spot to attach ):
			return;
		}
		
		//Set the memory location for our task stack
		m_TaskControl[m_TaskBlockIndex]._taskStack = _TASK_STACK_START_ADDRESS(m_TaskBlockIndex);
		
		//Set the task ID
		m_TaskControl[m_TaskBlockIndex].taskID = m_TaskBlockIndex;
		
		//Set the function
		m_TaskControl[m_TaskBlockIndex].task_func = taskPtr;
		
		//Default to scheduled
		m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_SCHEDULED;
		
		//Set our default timeout
		m_TaskControl[m_TaskBlockIndex].timeout = TASK_DEFAULT_TIMEOUT;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[m_TaskBlockIndex]._taskStack);
		m_TaskControl[m_TaskBlockIndex].taskExecutionContext.pc.ptr = taskPtr;

		//Increment our m_TaskBlockIndex
		m_TaskBlockIndex++;
		
		//Set our count to be equal to the m_TaskBlockIndex
		m_TaskBlockCount = m_TaskBlockIndex;
		
		
	}
}



/**
* \brief Attaches anything, but what should be a function, at the index passed. Can be dangerous if not used properly.
* \param taskPtr the address of what should be a function
* \param index The index to attach at
*/
void AttachTaskPointerAt(void *taskPtr, TaskIndiceType_t index)
{
	//If we have the ability to add a new block...
	if(index <= MAX_TASKS && index >= 0)
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
		m_TaskControl[index].task_func = taskPtr;
		
		//Default to scheduled
		m_TaskControl[index].taskStatus = TASK_SCHEDULED;
		
		//Set our default timeout
		m_TaskControl[index].timeout = TASK_DEFAULT_TIMEOUT;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[index].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[index]._taskStack);
		m_TaskControl[index].taskExecutionContext.pc.ptr = taskPtr;

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
	if(index < 0 || index >= MAX_TASKS)
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
* \brief Sets all tasks to be killed
*
*/
int8_t KillAllTasks()
{
	//Loop through all tasks and...
	for(TaskIndiceType_t i = 0; i <= MAX_TASKS; i++)
	{
		//Set the tasks status to kill
		m_TaskControl[i].taskStatus = TASK_KILL;
	}
	
	//Return 1
	return 1;
}



/**
* \brief Immediately kills all tasks
*
*/
int8_t _KillAllTasksImmediate()
{
	//Loop through all tasks and...
	for(TaskIndiceType_t index = 0; index <= MAX_TASKS; index++)
	{
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
	}
	
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
* \brief Sets all tasks to run, starts the schedulers interrupt service, and waits until all tasks are completed.
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
			//If we're within range for our count to set to ready and we meet all settings...
			if(i < m_TaskBlockCount && m_TaskControl[i].taskStatus != TASK_NONE)
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
		
		//Set our tasks running to true
		m_blnTasksRunning = true;
		
		//Launch the ISR
		_SCHEDULER_LAUNCH_ISR();
		
		//Re enable interrupts
		__asm__ __volatile__("sei \n\t":::"memory");
		
		//wait while our tasks are running
		while(m_blnTasksRunning == true);
		
		//Make sure our task index is reset back to 0 to allow us to more effectively 
		//launch tasks in the future. Covers some edge cases
		m_TaskBlockCount = 0;
	}

}



/**
* \brief Sets all tasks to run and starts the schedulers interrupt service. Keeps the final task as the passed task, especially useful for a kernel, imo
* \param mainfunc Function pointer to what will be the manager, kernel, or anything else you want classified as the 'main' task
*/
void StartTasks(void (*mainfunc)(void))
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
			//If we're within range for our count to set to ready and we meet all settings...
			if(i < m_TaskBlockCount && m_TaskControl[i].taskStatus != TASK_NONE)
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
		//Make sure last task is set, is reserved. Set its ID as well
		m_TaskControl[MAX_TASKS].taskStatus = TASK_MAIN;
		m_TaskControl[MAX_TASKS].taskID = MAX_TASKS;
		m_TaskControl[MAX_TASKS]._taskStack = _TASK_STACK_START_ADDRESS(MAX_TASKS);
		
		//Set the function
		m_TaskControl[MAX_TASKS].task_func = (void *)mainfunc;
		
		//Set our default timeout
		m_TaskControl[MAX_TASKS].timeout = TASK_DEFAULT_TIMEOUT;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[MAX_TASKS].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[MAX_TASKS]._taskStack + sizeof(TASK_STACK_SIZE) - 1);
		m_TaskControl[MAX_TASKS].taskExecutionContext.pc.ptr = (void *)mainfunc;
		
		//Set our current task to the last possible task, this way when entering for the first time we will loop to the first
		m_CurrentTask = &m_TaskControl[MAX_TASKS];
		
		//Set our tasks running to true
		m_blnTasksRunning = true;
		
		//Launch the ISR
		_SCHEDULER_LAUNCH_ISR();
		
		//Re enable interrupts
		__asm__ __volatile__("sei \n\t":::"memory");
		
		
		//wait while our tasks are running
		while(m_blnTasksRunning == true);
		
		//Make sure our task index is reset back to 0 to allow us to more effectively 
		//launch tasks in the future. Covers some edge cases
		m_TaskBlockCount = 0;
	}

}



/**
* \brief An empty task, also a good example for how to enter and exit a task appropriately.
*
*/
void _EmptyTask(void)
{
	//The current tasks ID
	TaskIndiceType_t tid = GetCurrentTask();
	
	//Task loop
	while (1)
	{
		
	}
	
	
	//Appropriate way to exit a task
	while(!KillTask(tid));
}



/**
* \brief Blocks all other tasks
*
*/
void BROKEN_TaskBlockOthers(TaskIndiceType_t tid)
{
	//Disable interrupts
	asm volatile("cli":::"memory");
	
	//Loop through all tasks and...
	for(TaskIndiceType_t i = 0; i < MAX_TASKS; i++)
	{
		//If it is not this task...
		if(i != tid)
		{
			TaskStatus_t currentStatus = m_TaskControl[i].taskStatus;
			
			if(currentStatus == TASK_READY)
			{
				//Set to blocked
				m_TaskControl[i].taskStatus = TASK_BLOCKED;
			}
			
				
		}
	}

	//Re-enable interrupts
	asm volatile("sei":::"memory");
	
	
}



/**
* \brief Frees all other tasks
*
*/
void BROKEN_TaskFreeOthers(TaskIndiceType_t tid)
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
* \param taskindex The index of the task, which if ran correctly should be the tasks ID
* \param counts The amount of counts to wait for
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
	
	//Make sure we're set to sleep
	m_TaskControl[taskIndex].taskStatus = TASK_SLEEP;
	
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
* \param counts The amount of counts to wait for
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
* \param taskIndex The index of the task to sleep, which if ran correctly should be the tasks ID
* \param counts The amount of counts to wait for
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
* \brief Requests data is copied from the source address to the destination address, yielding when unable to access
*/
uint8_t TaskYieldRequestDataCopy(TaskIndiceType_t id, void *memDestinationAddress, void *memSourceAddress, uint8_t bytes)
{
	uint8_t state = 0;
	
	while(m_semMemoryAccessor > 0)
	{
		TaskSetYield(id,10);
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor++;
	
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{	
		if((TaskMemoryLocationType_t)memSourceAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memSourceAddress) < RAMEND)
		{
			memcpy(memDestinationAddress, memSourceAddress, bytes);
			state = 1;
		}
		
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	return state;
}



/**
* \brief Requests data is copied from the source address to the destination address, exiting when unable to access
*/
uint8_t TaskRequestDataCopy(void *memDestinationAddress, void *memSourceAddress, uint8_t bytes)
{
	uint8_t state = 0;
	
	if(m_semMemoryAccessor > 0)
	{
		return 0;
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor++;
	
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{	
		if((TaskMemoryLocationType_t)memSourceAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memSourceAddress) < RAMEND)
		{
			memcpy(memDestinationAddress, memSourceAddress, bytes);
			state = 1;
		}
		
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	return state;
}



/**
* \brief Writes the data passed to memory passed directly, as opposed to using memcpy. Yields until the data can be written.
* \param id The tasks ID
* \param memDestinationAddress The place where to write data
* \param data The data to write
* \param bytes The amount of bytes to write
* \ret 0 is we were able to write, 1 if not
*/
uint8_t TaskYieldWriteData(TaskIndiceType_t id, void *memDestinationAddress, void *data, uint8_t bytes)
{
	uint8_t state = 0;
	
	if(m_semMemoryAccessor > 0)
	{
		TaskSetYield(id,10);
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor++;
	
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	
	
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{
		for(uint8_t i = 0; i < bytes; i++)
		{
			*((uint8_t *)(memDestinationAddress+i)) = *((uint8_t *)(data+i));
		}
		
		state = 1;
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	return state;
}



/**
* \brief Writes the data passed to memory passed directly, as opposed to using memcpy
* \param memDestinationAddress The place where to write data
* \param data The data to write
* \param bytes The amount of bytes to write
* \ret 0 is we were able to write, 1 if not
*/
uint8_t TaskRequestDataWrite(void *memDestinationAddress, void *data, uint8_t bytes)
{
	uint8_t state = 0;
	
	if(m_semMemoryAccessor > 0)
	{
		return 0;
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor++;
	
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	
	
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{
		for(uint8_t i = 0; i < bytes; i++)
		{
			*((uint8_t *)(memDestinationAddress+i)) = *((uint8_t *)(data+i));
		}
		
		state = 1;
	}
	
	//Disable Global interrupts
	__asm__ __volatile__("cli \n\t":::"memory");
	
	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	
	//Re enable interrupts
	__asm__ __volatile__("sei \n\t":::"memory");
	
	return state;
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
	
	//do this...
	do
	{
		//Increment our block index
		m_TaskBlockIndex++;
		
		//If our main task is set to the EMPTY function...
		if((TaskMemoryLocationType_t)m_TaskControl[MAX_TASKS].task_func == (TaskMemoryLocationType_t)_EmptyTask)
		{
			//Range check our block index
			if(m_TaskBlockIndex >= MAX_TASKS)
			{
				m_TaskBlockIndex = 0;
			}
		}
		//else...
		else
		{
			//Range check our block index
			if(m_TaskBlockIndex > MAX_TASKS)
			{
				m_TaskBlockIndex = 0;
			}
		}
		
		
		//If our safety is bad, meaning we have reached some form of catastrophic failure ): or all tasks have been killed ...
		if(--safety <= 0)
		{
			//Set our tasks running to false
			m_blnTasksRunning = false;
			
			//Make sure all tasks are dead
			for(TaskIndiceType_t i = 0; i <= MAX_TASKS; i++)
			{
				//Kill that damn task
				_KillTaskImmediate(i);
			}
			
			//Make sure our task counts are set to 0, allowing us to be able to launch tasks later
			m_TaskBlockCount = 0;
			
			//Make sure our schedulers ticking is stopped
			_SCHEDULER_STOP_TICK();
			
			//Break and return to wherever it is we go when tasks die.
			break;
		}
		
	}
	//While our tasks are either blocked, set to none, or are set to be killed
	while(m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_BLOCKED || m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_NONE || m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_KILL);
	


	//If our new process is not set to a blocked status...
	if(m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_BLOCKED && m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_NONE && m_TaskControl[m_TaskBlockIndex].taskStatus != TASK_KILL)
	{
		//if our task is set to be scheduled...
		if(m_TaskControl[m_TaskBlockIndex].taskStatus == TASK_SCHEDULED)
		{
			//Set as ready
			m_TaskControl[m_TaskBlockIndex].taskStatus = TASK_READY;
			
			//Go to our reserved task
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
		//Go to our reserved task
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
	//Make sure other interrupts are disabled
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


