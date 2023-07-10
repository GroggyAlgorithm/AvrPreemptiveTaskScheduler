/**
 * \file PreemptiveTaskSchedulerSharing.c
 * \author: Tim Robbins
 * \brief Source file for sharing resources in preemptive task scheduling and concurrent functionality. \n
 */
#include "PreemptiveTaskScheduler.h"
#include <string.h>

///An array block of our task control structures
static TaskControl_t m_TaskControl[MAX_TASKS+1] __attribute__ ((weakref));

///The index for our Task control block structure
static TaskIndiceType_t m_TaskBlockIndex __attribute__ ((weakref));

///Count of the total items we've placed into the task control block
static TaskIndiceType_t m_TaskBlockCount __attribute__ ((weakref));

///The current task context
volatile TaskControl_t *m_CurrentTask __attribute__ ((weakref));

///If the tasks have started running
static bool m_blnTasksRunning __attribute__ ((weakref));

///The type of task schedule to use
static TaskSchedule_t m_TaskSchedule __attribute__ ((weakref));

///Semaphore value for accessing memory, registers, ex. adc, etc.
static SemaphoreValueType_t m_semMemoryAccessor = 0;



typedef struct TaskMutex_t
{
	
	
	
}
/**
* \brief struct for mutex locking
*/
TaskMutex_t;



/**
* \brief Opens a request for the m_semMemoryAccessor
*
*/
uint8_t OpenRequest(bool waitForAccess)
{
	if(waitForAccess == true)
	{
		while(m_semMemoryAccessor > 0)
		{
			
		}
	}
	else
	{
		if(m_semMemoryAccessor > 0)
		{
			return 0;
		}
	}
	
	

	m_semMemoryAccessor++;
	
	
	return 1;
}



/**
* \brief Closes the request for m_semMemoryAccessor
*
*/
uint8_t CloseRequest()
{
	
	
	if(m_semMemoryAccessor <= 0)
	{
		m_semMemoryAccessor = 0;
		return 0;
	}
	else
	{
		m_semMemoryAccessor -= 1;
		return 1;
	}
}



/**
* \brief Requests data is copied from the source address to the destination address, yielding when unable to access
*/
uint8_t TaskYieldRequestDataCopy(TaskIndiceType_t id, void *memDestinationAddress, void *memSourceAddress, uint8_t bytes)
{
	uint8_t state = 0;
	
	while(m_semMemoryAccessor > 0)
	{
		TaskSetYield(id,5);
	}
	
	m_semMemoryAccessor++;
	
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{
		if((TaskMemoryLocationType_t)memSourceAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memSourceAddress) < RAMEND)
		{
			memcpy(memDestinationAddress, memSourceAddress, bytes);
			state = 1;
		}
		
	}
	
	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	
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
	
	m_semMemoryAccessor++;
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{
		if((TaskMemoryLocationType_t)memSourceAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memSourceAddress) < RAMEND)
		{
			memcpy(memDestinationAddress, memSourceAddress, bytes);
			state = 1;
		}
		
	}
	
	
	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	
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
	
	while(m_semMemoryAccessor > 0)
	{
		TaskSetYield(id,5);
	}
	

	m_semMemoryAccessor++;


	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{
		for(uint8_t i = 0; i < bytes; i++)
		{
			*((uint8_t *)(memDestinationAddress+i)) = *((uint8_t *)(data+i));
		}
		
		state = 1;
	}
	

	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}
	

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
	

	m_semMemoryAccessor++;

	
	
	if((TaskMemoryLocationType_t)memDestinationAddress < RAMEND && (bytes + (TaskMemoryLocationType_t)memDestinationAddress) < RAMEND)
	{
		for(uint8_t i = 0; i < bytes; i++)
		{
			*((uint8_t *)(memDestinationAddress+i)) = *((uint8_t *)(data+i));
		}
		
		state = 1;
	}
	

	m_semMemoryAccessor -= 1;
	
	if(m_semMemoryAccessor < 0)
	{
		m_semMemoryAccessor = 0;
	}

	return state;
}
