# AvrPreemptiveTaskScheduler
Preemptive task scheduling and concurrent functionality for Atmega, AVR, Arduino, and similar devices. 



Links to resources, influences, additional documents, and etc. can be found in the header files documentation.



Tested on the Atmega1284 in Atmel/Microchip Studio, have yet to try anything else but there shouldn't be much issues (in theory) and it should be compatible with the Arduino framework and the Arduino Uno, at least partially. If not, a config file can most likely take care of it, look over the header file and the macros and adjust definitions to meet interrupt needs.





The most likely problem for porting would be AVR specific instructions(cli, sei, etc.), the ISR keyword for interrupts and interrupt related definitions, non GCC based compilers, and a wildly different stack and general purpose register file structures. I plan to port to PIC for CAN communication.





<hr>

## Term Definitions
<br>

## Task

<br>
This file treats tasks as concurrent processing and uses the definition as such
<br>


### Mutex 
<br> 
A mutual exclusion lock that are used by one thread to stop concurrent access to protected data and resources
<br>


### Preemption
<br>
I'm at home and about to go to dinner, I'll add this later.
<br>



### Semaphore
<br>
A variable or abstract data type used to control access to a common resource

<hr>

## TODO

<br>


<br>

### Prioritizing Tasks
<br>
The big part, I need to add task priority levels for additional task ordering. Shouldn't be too hard, I just have to add a struct bit field variable and reorder the tasks upon each full iteration of the task collection...right? 
<br>


### Task arguments/params

<br>
The function args stopped working and now the id is not passing into its func ptr as it did earlier.
I'm not sure what I changed but previously the context switching passed additional arguments into the defined tasks but now, I assume it's the way I have the struct organized but I haven't tried very hard to fix this and I have had little need to do so since reading the task ID when entering the task works great.
<br>




### Circular array

<br>

Tasks are currently in a circular array but I would like to see them set up as pointers in a circular list, I think it can help with priority control when that's implemented.

<br>




### Classes...?

<br>


All of this, especially when related to the ID, may work better as a class object, using the context and control structs as members. Something to try, maybe, but this would make it more of a pain to port to microcontrollers without ready to use C++ support.



<hr>

<br>




## Example usage:

<br>

See Examples folder

<hr>

<br>
