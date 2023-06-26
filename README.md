# AvrPreemptiveTaskScheduler
Preemptive task scheduling and concurrent functionality on AVR


Tested on the Atmega1284, have yet to try anything else but there shouldn't be much issues (in theory)


<hr>

## Term Definitions
<br>

## Task

<br>
This file treats tasks as concurrent processing and uses the definition as such



### Mutex 
<br> 
A mutual exclusion lock that are used by one thread to stop concurrent access to protected data and resources



### Semaphore
<br>
A variable or abstract data type used to control access to a common resource

<hr>

## TODO

<br>

### Task arguments/params

<br>
The function args stopped working and now the id is not passing into its func ptr as it did earlier.
I'm not sure what I changed but previously the context switching passed additional arguments into the defined tasks but now, either the compiler optomized out or I've broken in some way

<br>

### Latency

<br>

Latency has been getting bad after so many tasks are scheduled. Using timer overflow interrupt 3, the timing has needed to change a lot o accommodate for this but it has been a little difficult to tune.

<br>

### MAX_TASKS

<br>

MAX_TASKS macro currently can not be set to be more than the amount of normal tasks scheduled. I tried to replace with things such as empty tasks but it just breaks and triggers the MCU's reset

<br>

### Circular array

<br>

Tasks are currently in a circular array but I would like to see them set up as pointers in a circular list, maybe to just get rid of the requirement for MAX_TASKS.

<br>


### Semaphores, Mutexs, and blocking

<br>

Blocking sometimes works, blocking sometimes doesn't. It seems to be related to the amount of tasks scheduled. The semaphores and mutex related things I've tried to setup does no always work, in a very annoying way. 


<br>


### Classes...?

<br>


All of this, especially when related to the ID, may work better as a class object, using the context and control structs as members. Something to try, maybe.



<hr>

<br>




## Example usage:

```
...
...
...
...


#include "PreemptiveTaskScheduler.h"


// Here, passing parameters no longer works, or at least did not when I tried last. Why? What happened?

static void Task1(TaskIndiceType_t id)
{
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

