/**
 * \file PreemptiveTaskScheduler.h
 * \author: Tim Robbins
 * \brief Preemptive task scheduling and concurrent functionality. \n
 *
 * For descriptions of definitions, refer to attached README.md  \n
 *
 * Links for influence and resources: \n
 * https://github.com/arbv/avr-context \n
 * https://github.com/kcuzner/kos-avr + http://kevincuzner.com/2015/12/31/writing-a-preemptive-task-scheduler-for-avr/ \n
 * https://gist.github.com/dheeptuck/da0a347358e60c77ea259090a61d78f4 \n
 * https://medium.com/@dheeptuck/building-a-real-time-operating-system-rtos-ground-up-a70640c64e93 \n
 * https://www.cs.princeton.edu/courses/archive/fall16/cos318/projects/project3/p3.html \n
 * https://www.nongnu.org/avr-libc/user-manual/inline_asm.html \n
 *
 */ 
#ifndef __PREEMPTIVETASKSCHEDULER_H__
#define __PREEMPTIVETASKSCHEDULER_H__	1



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */




#include "PreemptiveTaskSchedulerConfig.h"
#include "PreemptiveTaskSchedulerTypes.h"
#include "PreemptiveTaskSchedulerASM.h"


//FUNCTIONS-------------------------------------------------------------------------------------------



extern __attribute__ ((weak)) void _TaskSwitch(void);


//-----------------------------------
/*
I recommend looking these over if you use
*/
extern const TaskIndiceType_t _GetTaskID(TaskIndiceType_t index);
extern int8_t _KillTaskImmediate(TaskIndiceType_t index);
extern int8_t _KillAllTasksImmediate();
extern void _EmptyTask(void);
//-----------------------------------

extern uint8_t OpenRequest(bool waitForAccess);
extern uint8_t CloseRequest();
extern uint8_t TaskYieldRequestDataCopy(TaskIndiceType_t id, void *memDestinationAddress, void *memSourceAddress, uint8_t bytes);
extern uint8_t TaskRequestDataCopy(void *memDestinationAddress, void *memSourceAddress, uint8_t bytes);
extern uint8_t TaskYieldWriteData(TaskIndiceType_t id, void *memDestinationAddress, void *data, uint8_t bytes);
extern uint8_t TaskRequestDataWrite(void *memDestinationAddress, void *data, uint8_t bytes);
extern void SetTaskDefaultTimeout(TaskIndiceType_t id, TaskTimeout_t timeout);
extern void SetTaskSchedule(TaskSchedule_t schedule);
extern void SetTaskPriority(TaskIndiceType_t id, TaskPriorityLevel_t priority);
extern const bool AreTaskRunning();
extern const TaskIndiceType_t GetCurrentTaskID();
extern TaskIndiceType_t _GetTaskIndex(TaskIndiceType_t id);
extern TaskStatus_t GetTaskStatus(TaskIndiceType_t id);
extern void SetTaskStatus(TaskIndiceType_t id, TaskStatus_t status);
extern TaskIndiceType_t AttachTask(void *func, TaskIndiceType_t id);
extern int8_t KillTask(TaskIndiceType_t index);
extern int8_t KillAllTasks();
extern void DispatchTasks();
extern void StartTasks(void (*mainfunc)(void), TaskPriorityLevel_t taskPriority);
extern void TaskSleep(TaskIndiceType_t taskIndex, TaskTimeout_t counts);
extern void TaskYield(TaskTimeout_t counts);
extern void TaskSetYield(TaskIndiceType_t taskIndex, TaskTimeout_t counts);


///Helper for writing to a port, or a register, or whatever, when passing constant values. Ex. TaskWriteData(PORTB, 0x0F, writeStatus); 
#define TaskWriteData(_memDestination, _dataToWrite, _writeStatusOut)  do { typeof(_dataToWrite) _d = _dataToWrite; _writeStatusOut = TaskRequestDataWrite((void *)&_memDestination, &_d, sizeof(_d));  } while(0)

///Helper for writing to a port, or a register, or whatever, when passing constant values. Ex. TaskWaitForDataWrite(PORTB, 0x0F); 
#define TaskWaitForDataWrite(_memDestination, _dataToWrite)  do { typeof(_dataToWrite) _d = _dataToWrite; while(!TaskRequestDataWrite((void *)&_memDestination, &_d, sizeof(_d)));  } while(0)

///Helper for writing to a port, or a register, or whatever, when passing constant values. Ex. TaskYieldForDataWrite(taskID, PORTB, 0x0F); 
#define TaskYieldForDataWrite(_taskID, _memDestination, _dataToWrite)  do { typeof(_dataToWrite) _d = _dataToWrite; TaskYieldWriteData(_taskID, (void *)&_memDestination, &_d, sizeof(_d) );  } while(0)




/**
* \brief Schedules anything, but what should be a function, at the index passed. Can be dangerous if not used properly.
* \param taskPtr the address of what should be a function
* \param Returns the id for the scheduled task
*/
static TaskIndiceType_t ScheduleTaskPointer(void *taskPtr)
{
	TaskIndiceType_t nid = -1;
	volatile TaskStatus_t taskStatus = TASK_BLOCKED;
	
	for(nid = 0; nid < MAX_TASKS; nid++)
	{
		taskStatus = GetTaskStatus(nid);
		
		if(taskStatus == TASK_NONE)
		{
			AttachTask(taskPtr, nid);
			break;
		}
	}
	
	return nid;
}



/**
* \brief Adds and schedules a task to the available tasks for it to be ran when able
* \param func The function for running the task
* \param Returns the id for the scheduled task
*/
static TaskIndiceType_t ScheduleTask(void (*func)(void))
{
	TaskIndiceType_t nid = -1;
	volatile TaskStatus_t taskStatus = TASK_BLOCKED;
	
	for(nid = 0; nid < MAX_TASKS; nid++)
	{
		taskStatus = GetTaskStatus(nid);
		
		if(taskStatus == TASK_NONE)
		{
			AttachTask((void *)func, nid);
			break;
		}
	}
	
	return nid;
}


//----------------------------------------------------------------------------------------------------


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* __PREEMPTIVETASKSCHEDULER_H__ */
