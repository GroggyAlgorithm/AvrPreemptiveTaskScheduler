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

<br>

See Examples folder

<hr>

<br>
