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


///Atomic block for tasks, runs with no interrupts and restores post interrupt. When using this version, it is safer to use everywhere
#define TASK_CRITICAL_SECTION(...) __asm__ __volatile__("cli \n\t":::"memory"); __VA_ARGS__;  __asm__ __volatile__("sei \n\t":::"memory");

///Atomic block for tasks, runs with no interrupts and restores post interrupt, executing the arguments passed before entering the 'loop' but after disabling interrupts. it will break everything if used poorly.
#define TASK_CRITICAL_SECTION_LOCK(...) __asm__ __volatile__("cli \n\t":::"memory"); __VA_ARGS__; for(unsigned char __crit_locker __attribute__((__cleanup__(__iExitCritical))) = 1, __crit_blocker = 1; __crit_blocker != 0; __crit_blocker = 0)

///Block section for running with the schedulers switching disabled. It would be a very bad time to yield during here.
#define TASK_SWITCHING_LOCK(...) for(unsigned char __swit_locker __attribute__((__cleanup__(__iExitSchedulerPause))) = 1, __swit_blocker = __iEnterSchedulerPause(); __swit_blocker != 0; __swit_blocker = 0)

///Used for entering and exiting tasks, on enter the ID is gotten(you can reference through TaskSectionID) and on EXIT, Kills the task
#define TASK_SECTION(...) for(TaskIndiceType_t _______tid __attribute__((__cleanup__(__iExitTask))) = GetCurrentTaskID(), __blocker = 1; __blocker != 0 && _______tid >= 0; __blocker = 0)

///Used for entering and exiting tasks but includes forever loop, on enter the ID is gotten(you can reference through TaskSectionID) and on EXIT, Kills the task. executes the arguments passed before entering the 'loop'
#define TASK_RUN(...) __VA_ARGS__; for(TaskIndiceType_t _______rid __attribute__((__cleanup__(__iExitTask))) = GetCurrentTaskID(), __runner_blocker = 1; __runner_blocker != 0 && _______rid >= 0;) while(__runner_blocker != 0)

///Used for the semaphore request block, executes any passed arguments before requesting
#define TASK_SEM_REQUEST_BLOCK(...) __VA_ARGS__; for(TaskIndiceType_t _____sem __attribute__((__cleanup__(__iExitSem))) = OpenSemaphoreRequest(true); _____sem;)



///Macro reference to the ID gotten during the current task sections
#define TaskSectionID		_______tid
#define TaskRunID			_______rid

///Helper for exiting a task when using the task section thing
#define TaskRunExit			__runner_blocker = 0; break;










//FUNCTIONS-------------------------------------------------------------------------------------------



extern __attribute__ ((weak)) void _TaskSwitch(void);



extern TaskControl_t* GetTask(void *task_func);
extern TaskPriorityLevel_t FindNextHighestPriorityLevel();
extern TaskIndiceType_t FindNextHighestPriorityTask();
extern TaskIndiceType_t FindNextPriorityTask();
extern uint8_t OpenSemaphoreRequest(bool waitForAccess);
extern uint8_t CloseSemaphoreRequest();
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
extern void StartTasks(void *mainfunc, TaskPriorityLevel_t taskPriority);
extern void TaskSleep(TaskIndiceType_t taskIndex, TaskTimeout_t counts);
extern void TaskSetYield(TaskIndiceType_t taskIndex, TaskTimeout_t counts);

//-----------------------------------

/*
I highly recommend looking these over if you use
*/
extern const TaskIndiceType_t _GetTaskID(TaskIndiceType_t index);
extern int8_t _KillTaskImmediate(TaskIndiceType_t index);
extern int8_t _KillAllTasksImmediate();
extern void _EmptyTask(void);

//-----------------------------------


extern int8_t KillOtherTasks(TaskIndiceType_t tid);
extern TaskIndiceType_t GetActiveTaskCount();
extern const TaskIndiceType_t GetTaskBlockCount();
extern bool IsTaskActive(TaskIndiceType_t tid);

/**
* \brief Schedules anything, but what should be a function, at the index passed. Can be dangerous if not used properly.
* \param taskPtr the address of what should be a function
* \param Returns the id for the scheduled task
*/
__attribute__ ((unused)) static TaskIndiceType_t ScheduleTaskPointer(void *taskPtr) 
{
	TaskIndiceType_t nid = -1;
	volatile TaskStatus_t taskStatus = TASK_BLOCKED;
	
	for(nid = 0; nid < MAX_TASKS; nid++)
	{
		TASK_CRITICAL_SECTION ( taskStatus = GetTaskStatus(nid); );
		
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
__attribute__ ((unused)) static TaskIndiceType_t ScheduleTask(void (*func)(void))
{
	TaskIndiceType_t nid = -1;
	volatile TaskStatus_t taskStatus = TASK_BLOCKED;
	
	for(nid = 0; nid < MAX_TASKS; nid++)
	{
		
		TASK_CRITICAL_SECTION ( taskStatus = GetTaskStatus(nid) );
		
		if(taskStatus == TASK_NONE)
		{
			AttachTask((void *)func, nid);
			break;
		}
	}
	
	return nid;
}













static __attribute__((always_inline)) inline void __iExitTask(const TaskIndiceType_t *__s)
{
	while(KillTask(*__s));
}
static __attribute__((always_inline)) inline unsigned char __iEnterCritical(void)
{
	__asm__ __volatile__("cli \n\t":::"memory");
	return 1;
}
static __attribute__((always_inline)) inline void __iExitCritical(const unsigned char *__s)
{
	__asm__ volatile ("sei \n\t" ::: "memory");
}
static __attribute__((always_inline)) inline void __iExitSem(const unsigned char *__s)
{
	CloseSemaphoreRequest();
}
static __attribute__((always_inline)) inline unsigned char __iEnterSchedulerPause(void)
{
	_SCHEDULER_STOP_TICK();
	return 1;
}
static __attribute__((always_inline)) inline void __iExitSchedulerPause(const unsigned char *__s)
{
	_SCHEDULER_START_TICK();
}






//----------------------------------------------------------------------------------------------------







#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* __PREEMPTIVETASKSCHEDULER_H__ */
