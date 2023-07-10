/**
 * \file PreemptiveTaskScheduler.c
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and concurrent functionality.
 */
#include "PreemptiveTaskScheduler.h"

#ifdef __AVR
#include <avr/interrupt.h>
#endif


///An array block of our task control structures
static TaskControl_t m_TaskControl[MAX_TASKS+1];

///The index for our Task control block structure
static TaskIndiceType_t m_TaskBlockIndex = 0;

///Count of the total items we've placed into the task control block
static TaskIndiceType_t m_TaskBlockCount = 0;

///The current task context
volatile TaskControl_t *m_CurrentTask;

///If the tasks have started running
static bool m_blnTasksRunning = false;

///The type of task schedule to use
static TaskSchedule_t m_TaskSchedule = TASK_SCHEDULE_ROUND_ROBIN;



/**
* \brief Sets the default timeout for the task at the ID
* \param id The task to set
* \param timeout The value that after each timeout countdown, the timeout should reset to
*/
inline void SetTaskDefaultTimeout(TaskIndiceType_t id, TaskTimeout_t timeout)
{
	//If our ID is within range...
	if(id >= 0 && id <= MAX_TASKS)
	{
		//Set our priority
		m_TaskControl[id].defaultTimeout = timeout;
	}
}



/**
* \brief Deep copy from src to dest
*
*/
void _TaskCpy(TaskControl_t *dest, TaskControl_t *src)
{
	dest->defaultTimeout = src->defaultTimeout;
	dest->priority = src->priority;
	dest->task_func = src->task_func;
	dest->taskData = src->taskData;
	dest->taskExecutionContext = src->taskExecutionContext;
	
	dest->taskExecutionContext.sreg = src->taskExecutionContext.sreg;
	dest->taskExecutionContext.sp.ptr = src->taskExecutionContext.sp.ptr;
	dest->taskExecutionContext.pc.ptr = src->taskExecutionContext.pc.ptr;
	
	for(uint8_t i = 0; i < TASK_REGISTERS; i++)
	{
		dest->taskExecutionContext.registerFile[i] = src->taskExecutionContext.registerFile[i];
	}
	
	
	dest->taskID = src->taskID;
	dest->taskStatus = src->taskStatus;
	dest->timeout = src->timeout;
	dest->_taskStack = src->_taskStack;
}



/**
* \brief Swaps tasks a and b
*
*/
void _MemSwapTasks(TaskControl_t *taskA, TaskControl_t *taskB)
{
	//Intermediary value. Too many "complex" data types to easily use the ^ option ):
	TaskControl_t tmpTask;
	
	_TaskCpy(&tmpTask, taskA);
	_TaskCpy(taskA, taskB);
	_TaskCpy(taskB, &tmpTask);
}



/**
* \brief Sets the priority level of the task with the passed ID
* \param id The id of the task
* \param priority The priority level for the task at the id. The higher the priority value, the higher the tasks priority
*/
inline void SetTaskPriority(TaskIndiceType_t id, TaskPriorityLevel_t priority)
{
	//If our ID is within range...
	if(id >= 0 && id < MAX_TASKS)
	{
		//Set our priority
		m_TaskControl[id].priority = priority;
	}
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
* \brief Sets all tasks to run and starts the schedulers interrupt service. Keeps the final task as the passed task, especially useful for a kernel, synchronizer, empty spacer tasks, etc, imo
* \param mainfunc Function pointer to what will be the manager, kernel, empty task, or anything else you want classified as the 'main' task
*/
void StartTasks(void (*mainfunc)(void), TaskPriorityLevel_t taskPriority)
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
		m_TaskControl[MAX_TASKS].timeout = 0;
		m_TaskControl[MAX_TASKS].defaultTimeout = 0;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[MAX_TASKS].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[MAX_TASKS]._taskStack);
		m_TaskControl[MAX_TASKS].taskExecutionContext.pc.ptr = (void *)mainfunc;
		
		//Set the max tasks control to the passed priority level
		m_TaskControl[MAX_TASKS].priority = taskPriority;
		
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
* \brief Attaches a task to the available tasks
* \param func The function for running the task
* \param id The position to attach at as well as the tasks ID
* \ret The next id/index position
*/
//TaskIndiceType_t AttachTaskAt(void (*func)(void), TaskIndiceType_t index)
TaskIndiceType_t AttachTask(void *func, TaskIndiceType_t id)
{
	//If we have the ability to add a new block...
	if(id < MAX_TASKS && id >= 0)
	{
		
		//Check for allowances
		if((TaskMemoryLocationType_t)_TASK_STACK_START_ADDRESS(id) < RAMSTART)
		{
			return id;	
		}
		
		//Set the memory location for our task stack
		m_TaskControl[id]._taskStack = _TASK_STACK_START_ADDRESS(id);
				
		
		//Set the task ID
		m_TaskControl[id].taskID = id;
		
		//Set the function
		m_TaskControl[id].task_func = func;
		
		//Default to scheduled
		m_TaskControl[id].taskStatus = TASK_SCHEDULED;
		
		//Set our default timeouts
		m_TaskControl[id].timeout = 0;
		m_TaskControl[id].defaultTimeout = 0;
		
		//initialize stack pointer and program counter for our program execution
		m_TaskControl[id].taskExecutionContext.sp.ptr = ((uint8_t *)m_TaskControl[id]._taskStack);
		m_TaskControl[id].taskExecutionContext.pc.ptr = func;
		
		//Set our default priority
		m_TaskControl[id].priority = 0;
		
		//Increment our id
		id++;
		
		//Set our count to be equal to the id
		m_TaskBlockCount = id;
		
	}
	
	
	return id;
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
	m_TaskControl[index].priority = 0;
	
	
	//Reset our timeouts
	m_TaskControl[index].timeout = 0;
	m_TaskControl[index].defaultTimeout = 0;
	
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
		m_TaskControl[index].priority = 0;
		
		//Reset our timeouts
		m_TaskControl[index].timeout = 0;
		m_TaskControl[index].defaultTimeout = 0;

	
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
* \brief Sets this task to sleep for the specified amount of counts, counting down here locally
* \param taskIndex The index of the task, which if ran correctly should be the tasks ID
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
	
	//Save our tasks status
	volatile TaskStatus_t savedStatus = m_TaskControl[taskIndex].taskStatus;
	
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
	
	//Revert back to our saved status before exiting
	m_TaskControl[taskIndex].taskStatus = savedStatus;
	
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
* \brief Sets all tasks to run, starts the schedulers interrupt service, and waits until all tasks are completed.
*
*/
inline void DispatchTasks()
{
	//Call the start tasks, using the "empty task" as the main task and a very low priority level
	StartTasks(_EmptyTask, 0);
}



/**
* \brief Sets the type of scheduling used
*
*/
inline void SetTaskSchedule(TaskSchedule_t schedule)
{
	m_TaskSchedule = schedule;
}



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
const inline TaskIndiceType_t GetCurrentTaskID()
{
	//Return the id to our pointers current task id
	return m_CurrentTask->taskID;
}



/**
* \brief An empty task, also a good example for how to enter and exit a task appropriately.
*
*/
void _EmptyTask(void)
{
	//The current tasks ID
	TaskIndiceType_t tid = GetCurrentTaskID();
	
	//Task loop
	while (1)
	{
		
	}
	
	
	//Appropriate way to exit a task
	while(!KillTask(tid));
}




#include "PreemptiveTaskSchedulerSwitching.c"


