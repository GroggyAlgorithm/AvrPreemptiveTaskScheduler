/**
 * \file PreemptiveTaskSchedulerSwitching.c
 * \author: Tim Robbins
 * \brief Source file for preemptive task scheduling and concurrent context switching functionality. \n
 */
#include "PreemptiveTaskScheduler.h"




///An array block of our task control structures
static TaskControl_t m_TaskControl[MAX_TASKS+1] __attribute__ ((weakref));

///The index for our Task control block structure
static TaskIndiceType_t m_TaskBlockIndex __attribute__ ((weakref));

///Count of the total items we've placed into the task control block
static TaskIndiceType_t m_TaskBlockCount __attribute__ ((weakref));

///The current task context
volatile TaskControl_t *m_CurrentTask __attribute__ ((weakref));

///Semaphore value for accessing memory, registers, ex. adc, etc.
static SemaphoreValueType_t m_semMemoryAccessor __attribute__ ((weakref));

///If the tasks have started running
static bool m_blnTasksRunning __attribute__ ((weakref));

///The type of task schedule to use
static TaskSchedule_t m_TaskSchedule __attribute__ ((weakref));





/**
* \brief Reorders the task control collection based on priority settings
*
*/
__attribute__ ((weak)) void _PriorityReorderTasks(void)
{
	//Loop through all tasks and...
	for(TaskIndiceType_t i = 1; i < MAX_TASKS; i++)
	{
		//If our current priority level is set higher than our previous priority level...
		if(m_TaskControl[i].priority > m_TaskControl[i-1].priority)
		{
			//Loop through all tasks and...
			for(TaskIndiceType_t j = i; j > 0; j--)
			{				
				//If our current priority level is set higher than our previous priority level...
				if(m_TaskControl[j].priority > m_TaskControl[j-1].priority)
				{
					//Move to the front
					_MemSwapTasks(&m_TaskControl[j], &m_TaskControl[j-1]);
				}
				//else...
				else
				{
					//break outta here
					break;
				}
			}
		}
	}
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
		//Kill it
		_KillTaskImmediate(m_TaskBlockIndex);
	}
	
	//Loop through all tasks and...
	for(TaskIndiceType_t i = 0; i <= MAX_TASKS; i++)
	{
		//If the task is in a state that allows us...
		if(m_TaskControl[i].taskStatus != TASK_BLOCKED && m_TaskControl[i].taskStatus != TASK_NONE && m_TaskControl[i].taskStatus != TASK_SLEEP)
		{
			//If our task counts are greater than 0...
			if(m_TaskControl[i].timeout > 0)
			{
				
				//If Decrementing our count is now less than or equal to 0...
				m_TaskControl[i].timeout -= 1;
				
				if(m_TaskControl[i].timeout <= 0)
				{
					//If our task is set to YIELD...
					if(m_TaskControl[i].taskStatus == TASK_YIELD)
					{
						//Set task back to ready
						m_TaskControl[i].taskStatus = TASK_READY;
					}
					
					//Reset to our default timeout
					m_TaskControl[i].timeout = m_TaskControl[i].defaultTimeout;
				}
			}
			
		}
	}
	
	
	//If we're at our first task...
	if(m_TaskBlockIndex == 0)
	{
		//Check our schedule type
		switch (m_TaskSchedule)
		{
			case TASK_SCHEDULE_PRIORITY:
				_PriorityReorderTasks();
			break;
			
			default:
			break;
		};
		
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
SCHEDULER_INTERRUPT_KEYWORD(SCHEDULER_INT_VECTOR, ISR_NAKED)
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