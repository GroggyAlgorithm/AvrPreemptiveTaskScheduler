# AvrPreemptiveTaskScheduler
Preemptive task scheduling and concurrent functionality on AVR



Tested on the Atmega1284 in Atmel/Microchip Studio, have yet to try anything else but there shouldn't be much issues (in theory) and it should be compatible with the Arduino framework and the Arduino Uno, at least partially. if not, a config file can most likely take care of it, look over the header file and the macros.



<hr>

## Term Definitions
<br>

## Task

<br>
This file treats tasks as concurrent processing and uses the definition as such



### Mutex 
<br> 
A mutual exclusion lock that are used by one thread to stop concurrent access to protected data and resources



### Preemtion
<br>
I'm at home and about to go to dinner, I'll add this later.




### Semaphore
<br>
A variable or abstract data type used to control access to a common resource

<hr>

## TODO

<br>

### Task arguments/params

<br>
The function args stopped working and now the id is not passing into its func ptr as it did earlier.
I'm not sure what I changed but previously the context switching passed additional arguments into the defined tasks but now, I assume it's the way I have the struct organized.

<br>


### Circular array

<br>

Tasks are currently in a circular array but I would like to see them set up as pointers in a circular list, I think it can help with priority control when that's implemented.

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
