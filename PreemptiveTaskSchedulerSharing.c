/**
 * \file PreemptiveTaskSchedulerSharing.c
 * \author: Tim Robbins
 * \brief Source file for sharing resources in preemptive task scheduling and concurrent functionality. \n
 */
#include "PreemptiveTaskScheduler.h"



///Semaphore value for accessing memory, registers, ex. adc, etc.
static SemaphoreValueType_t m_semAccessor = 0;


/**
* \brief Opens a request for the accessor
*
*/
uint8_t OpenSemaphoreRequest(bool waitForAccess)
{
	
	if(waitForAccess == true)
	{
		while(m_semAccessor > 0)
		{
			
		}
	}
	else
	{
		if(m_semAccessor > 0)
		{
			return 0;
		}
	}
	
	

	TASK_CRITICAL_SECTION ( m_semAccessor++; );
	
	
	return 1;
}



/**
* \brief Closes the request for the semaphore accessor
*
*/
uint8_t CloseSemaphoreRequest()
{
	if(m_semAccessor <= 0)
	{
		TASK_CRITICAL_SECTION ( m_semAccessor = 0; );
		return 0;
	}
	else
	{
		TASK_CRITICAL_SECTION ( m_semAccessor -= 1; );
		return 1;
	}
}



