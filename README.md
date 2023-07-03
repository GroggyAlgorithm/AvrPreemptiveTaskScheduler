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


### Preemption and Preemptive Scheduling
<br>
In computing, preemption is the act of temporarily interrupting an executing task, with the intention of resuming it at a later time.
<br>



### Semaphore
<br>
A variable or abstract data type used to control access to a common resource

<hr>

## TODO

<br>


<br>




### Classes...?

<br>


All of this, especially when related to the ID, may work better as a class object, using the context and control structs as members. Something to try, maybe, but this would make it more of a pain to port to microcontrollers without ready to use C++ support.



<hr>

<br>




## Example usage:

<br>


### Some addition usage info: 
<br>

Setting task priority along with default timeouts can really change the way prioritization and scheduling can happen.

<br>

It's best to grab the tasks ID on the way into the tasks function and to always add a task kill loop at the end of the function.

<br>

Setting data through the provided functions can be best to avoid access conflicts, especially when it can take multiple cycles, but it is not always required. User discretion is advised.

<br>

See Examples folder

<hr>

<br>
