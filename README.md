# AvrPreemptiveTaskScheduler
Preemptive task scheduling and async functionality on AVR



```

/**
 * \file PreemptiveTaskScheduler.h
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and async functionality. \n
 *
 * Terms to know \n
 *
 * 
 *
 *
 * Semaphore, from Wikipedia(https://en.wikipedia.org/wiki/Semaphore_(programming): \n
 *
 * In computer science, a semaphore is a variable or abstract data type used to control access to \n
 * a common resource by multiple threads and avoid critical section problems in a concurrent system \n
 * such as a multitasking operating system. Semaphores are a type of synchronization primitive. \n
 * A trivial semaphore is a plain variable that is changed (for example, incremented or decremented, or toggled) \n
 * depending on programmer-defined conditions.
 *
 * A useful way to think of a semaphore as used in a real-world system is as a record of how many units of \n
 * a particular resource are available, coupled with operations to adjust that record safely (i.e., to avoid race conditions) \n
 * as units are acquired or become free, and, if necessary, wait until a unit of the resource becomes available. \n
 * \n
 * \n
 * \n
 * This file treats tasks as async functionality and uses the definition as such \n
 * 
 *
 *
 * Links for influence and resources: \n
 * https://github.com/arbv/avr-context \n
 * https://github.com/kcuzner/kos-avr + http://kevincuzner.com/2015/12/31/writing-a-preemptive-task-scheduler-for-avr/ \n
 * https://gist.github.com/dheeptuck/da0a347358e60c77ea259090a61d78f4 \n
 * https://medium.com/@dheeptuck/building-a-real-time-operating-system-rtos-ground-up-a70640c64e93 \n
 *
 * 
 *
 *
 * \todo \n
 * Task latency gets pretty bad after so many tasks are scheduled and the timing needs changed to accommodate and is difficult to tune \n
 * Sometimes blocking works, sometimes it don't  \n
 * The function args stopped working and now the id is not passing into its func ptr as it did earlier. \n
 * MAX_TASKS currently can not be set to be more than the amount of tasks added. \n
 * Tasks are currently in a circular array, would like to switch to using pointers \n
 * Semaphores for shared resources and locations, main task for handling resource requests, task sync for resource handling \n
 *
 *
 *
 */ 

```



<hr>

<br>




Example usage:

```
...
...
...
...


#include "PreemptiveTaskScheduler.h"

static void Task1(TaskIndiceType_t id)
{
	volatile TaskIndiceType_t dbugid = id;
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 7);
			TaskYield(100);	
		}
		
		
		
	}
	
}


static void Task2(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 6);
			TaskSetYield(tid,1000);
		}
		
		
	}
}


static void Task3(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 5);
			TaskSetYield(tid, 500);
		}
	}
}



static void Task4(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 4);
			TaskSetYield(tid, 750);
		}
	}
}

static void Task5(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 3);
			TaskSetYield(tid, 400);
		}
	}
}

static void Task6(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 2);
			TaskSetYield(tid, 250);
		}
	}
}

static void Task7(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 1);
			TaskSetYield(tid, 851);
		}
	}
}

static void Task8(void)
{
	volatile TaskIndiceType_t tid = GetCurrentTask();
	
	while(1)
	{
		if(GetTaskStatus(tid) == TASK_READY)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 0);
			TaskSetYield(tid, 121);
		}
	}
}



int main(void)
{
  DDRD = 0xff;
	PORTD |= 0xff;

	AttachIDTask(Task1);
	AttachTask(Task2);
	AttachTask(Task3);
	AttachTask(Task4);
	AttachTask(Task5);
	AttachTask(Task6);
	AttachTask(Task7);
	AttachTask(Task8);
	
	DispatchTasks();
  
  while(1)
  {

  }
}

```

