﻿/**
 * \file ExampleMain.cpp
 * \author Tim Robbins
 * \date 5/4/2023
 *
 * \brief Example usage of the preemptive task scheduler. \n
 * PORTA:0 is connected to 5v (connect to ground to see a change) and PORTA:1 is connected to ground \n
 * PORTD and PORTC:7 is connected to LED's with 300 ohm pull down resistors \n
 * PORTB will also run through each pin setting high and then low \n
 * Created using the Atmega1284, uses PORTA for adc, PORTD for LED blinking, and PORTC:7 for blinking. 12Mhz external crystal. \n
 */ 

///The frequency being used for the controller
#define F_CPU                                       12000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <stdio.h>

///The amount of max tasks we're allowed
#define MAX_TASKS				11

#define SCHEDULER_INT_VECTOR	TIMER3_OVF_vect

#define TASK_INTERRUPT_TICKS	0x1f0

#include "PreemptiveTaskScheduler.h"
#include "mcu_lib/mcuDelays.h"
//------------------------------------------------------------------

///Voltage reference mode 1
#define ADC_REF_MODE_1							(0b00 << REFS0)

///Voltage reference mode 2
#define ADC_REF_MODE_2							(0b01 << REFS0)

///Voltage reference mode 3
#define ADC_REF_MODE_3							(0b10 << REFS0)

///Voltage reference mode 4
#define ADC_REF_MODE_4							(0b11 << REFS0)

//------------------------------------------------------------------


//Variables---------------------------------------------------------

///Example adc values
static uint16_t m_AdcValues[8] = {0};
	
//------------------------------------------------------------------

#define verbatim __attribute__((always_inline)) inline
#define static_extern	__attribute__ ((weakref))

//Functions---------------------------------------------------------

void AdcSetup();
uint16_t SampleAdc(uint8_t adcChannel, uint8_t sampleCount);
static void QuitableTask(void);
static void QuitableTask2(void);
static void Task0();
static void Task1(void);
static void Task2(void);
static void Task3(void);
static void Task4(void);
static void Task5(void);
static void Task6(void);
static void Task7(void);
static void Task8(void);
static void Task9(void);
static void AdcGetter(void);
static uint16_t TaskReadAdc(TaskIndiceType_t tid, uint8_t adcChannel);

//------------------------------------------------------------------



/**
* \brief Drop in point. Change name and call in main if that's better
* ExampleMain, main
*/
int main(void)
{
	//Initialize ADC
	AdcSetup();
	
	//Initialize other ports
	DDRD = 0xff;
	DDRC = 0xff;
	DDRB = 0xff;
	
	PORTD |= 0xff;
	
	//Short delay, just in case (:
	_delay_ms(10);
	
	
	//Schedule our quitable tasks
	ScheduleTask(QuitableTask);
	ScheduleTask(QuitableTask2);
	

	//Dispatch the tasks
	DispatchTasks();

	
	//Once finished, Schedule our other tasks
	ScheduleTask(Task0);
	ScheduleTask(Task1);
	ScheduleTask(Task2);
	ScheduleTask(Task3);
	ScheduleTask(Task4);
	ScheduleTask(Task5);
	ScheduleTask(Task6);
	ScheduleTask(Task7);
	ScheduleTask(AdcGetter);
	
	#warning setting tasks by priority does not seem to work the best. Sometimes, it breaks everything.
	SetTaskSchedule(TASK_SCHEDULE_PRIORITY);
	
	SetTaskPriority(0, 1);
	SetTaskPriority(1, 5);
	SetTaskPriority(2, 2);
	SetTaskPriority(4, 3);
	SetTaskPriority(5, 4);
	SetTaskPriority(3, 5);
	
	//Dispatch the tasks
	DispatchTasks();
	
	
	
	//Loop forever in case something goes wrong
	while(1)
	{
		for(uint8_t i = 0; i < 10; i++)
		{
			PORTD ^= 0xff;
			PORTC ^= (1 << 7);
			_delay_ms(100);
		}
		
		
	}
	
}



/**
* \brief Initializes ADC for the Atmega1284
*
*/
void AdcSetup()
{
	//Set as input low, internal pull up disabled
	PORTA = 0x00;
	DDRA = 0;
	
	//Disable digital input
	DIDR0 |= 0xff;
	
	//Make sure to right justify results
	ADMUX &= ~(1 << ADLAR);
	
	//Set for free running mode
	ADCSRB &= ~(1 << ADTS0 | 1 << ADTS1 | 1 << ADTS2); 
	
	//Set admux mode
	ADMUX |= ADC_REF_MODE_1;
	
	//If we're actually using a controller with AREFEN bits (Atmega64m1 iirc)...
	#if defined(AREFEN)
		//Make sure to enable it
		ADCSRB |= (1 << AREFEN);
	#endif
	
	//Turn on ADC
	ADCSRA |= (1 << ADEN);
}



/**
* \brief Samples ADC for sampleCount Counts on channel adcChannel, returning the average
*
*/
uint16_t SampleAdc(uint8_t adcChannel, uint8_t sampleCount)
{
	//Variables
    uint16_t adcResult = 0; //Return value from the ADC
    
    //If the sample count is greater than 0, avoid divide by 0 errors,...
    if(sampleCount > 0)
    {
        //Create a variable for the loop
        uint8_t i = 0;

        //Select the passed channel
        ADMUX |= adcChannel;

        //While the index is less than the passed sample count...
        while(i < sampleCount)
        {
            i++;

            //Start our conversion
			ADCSRA |= (1 << ADSC);
			
			//Wait until the conversion is finished
            while(((ADCSRA >> ADSC) & 0x01));
			
			//Add our result
            adcResult += ADC;
        }
        
		//Divide our result by the passed sample count
        adcResult /= sampleCount;

        //Make sure the clear the selected channel on the way out
        ADMUX &= ~adcChannel;

    }
    
	//Return our result
    return adcResult;
}



/**
* \brief A task that exits after performing something
*/
static void QuitableTask(void)
{
	TASK_SECTION()
	{
		for(uint8_t i = 0; i < 10; i++)
		{
			PORTD ^= 0xff;
			TaskSetYield(TaskSectionID,1500);
		}
	}
}



/**
* \brief A task that exits after performing something
*/
static void QuitableTask2(void)
{
	TASK_SECTION()
	{
		for(uint8_t i = 0; i < 5; i++)
		{
			PORTC ^= (1 << 7);
			TaskSetYield(TaskSectionID,1000);
		}
	}
}



/**
* \brief Task that forever reads the saved ADC:0 value and blinks D:7 on a variable time frame if above a certain adc reading, much faster if not.
*/
static void Task0()
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	uint16_t adcValue = 0;
	uint16_t counter = 150;
	bool countDir = false;
	
	while(1)
	{
		adcValue = TaskReadAdc(tid, 0);
		
		if(adcValue > 700)
		{
			__asm__ __volatile__("nop");
			PORTD ^= (1 << 7);
			TaskSetYield(tid,counter);
			
			if(countDir == true)
			{
				if(counter >= 150)
				{
					countDir = false;
				}
				else
				{
					counter++;
				}
			}
			else
			{
				if(counter <= 10)
				{
					countDir = true;
				}
				else
				{
					counter--;
				}
			}
		}
		else
		{
			for (uint8_t i = 0; i < 40; i++)
			{
				PORTD ^= (1 << 7);
				TaskSetYield(tid,20);
			}
		}
		
		
		
		
		
	}
	
	//Exit if we reached here
	while(!KillTask(tid));
	
}



/**
* \brief Task that forever reads the saved ADC:1 value and blinks D:6 slower if below a certain adc reading, much faster if not.
*/
static void Task1(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	uint8_t counter = 0;
	uint16_t adcValue = 0;
	TaskIndiceType_t savedTN = 0;
	
	
	while(1)
	{
		adcValue = TaskReadAdc(tid, 1);
		
		if(adcValue > 700)
		{
			for (uint8_t i = 0; i < 10; i++)
			{
				PORTD ^= (1 << 6);
				TaskSetYield(tid,50);
			}
		}
		
		else
		{
			TaskSetYield(tid,500);
			PORTD |= (1 << 6);
			TaskSetYield(tid,500);
			PORTD &= ~(1 << 6);
		}
		
		
	}
	
	//Exit if we reached here
	while(!KillTask(tid));
}



/**
* \brief Task that forever blinks D:5
*/
static void Task2(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	
	while(1)
	{
		__asm__ __volatile__("nop");
		PORTD ^= (1 << 5);
		TaskSetYield(tid, 500);
	}
	
	//Exit if we reached here
	while(!KillTask(tid));

}



/**
* \brief Task that forever blinks D:4 and PORTB
*/
static void Task3(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	uint8_t pout = 0x01;
	uint8_t testStatus = 0;
	while(1)
	{
		
			TASK_CRITICAL_SECTION_LOCK()
			{
				PORTB ^= 0xff;
				TaskSetYield(tid, 750);		
			}
			
			__asm__ __volatile__("nop");
			
			TASK_CRITICAL_SECTION_LOCK()
			{
				PORTB ^= 0xff;
				TaskSetYield(tid, 375);
			}
			
			__asm__ __volatile__("nop");
			
			TASK_CRITICAL_SECTION_LOCK()
			{
				PORTD ^= (1 << 4);
				TaskSetYield(tid, 375);
			}
						
			__asm__ __volatile__("nop");
	}
	
	//Exit if we reached here
	while(!KillTask(tid));
}



/**
* \brief Task that forever blinks D:3
*/
static void Task4(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	
	while(1)
	{
		__asm__ __volatile__("nop");
		PORTD ^= (1 << 3);
		TaskSetYield(tid, 400);
	}
	
	//Exit if we reached here
	while(!KillTask(tid));
}



/**
* \brief Task that forever blinks D:2
*/
static void Task5(void)
{
	
	TaskIndiceType_t tid = GetCurrentTaskID();
	
	while(1)
	{
		__asm__ __volatile__("nop");

		TASK_CRITICAL_SECTION_LOCK()
		{
			PORTD ^= (1 << 2);
			TaskSetYield(tid, 250);
		}
		
	}
	
	//Exit if we reached here
	while(!KillTask(tid));
}



/**
* \brief Task that forever blinks D:1
*/
static void Task6(void)
{
	TASK_RUN()
	{	
		__asm__ __volatile__("nop");
		PORTD ^= (1 << 1);
		TaskSetYield(TaskRunID, 851);
		
	}
}



/**
* \brief Task that forever blinks D:0 and schedules task 8. Use debugger and step through or change the flag to see things going on
*/
static void Task7(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	TaskIndiceType_t subTid = -1;
	bool hasBeenSet = false;
	while(1)
	{
		if(hasBeenSet == false)
		{
			
			
			hasBeenSet = true;
			subTid = ScheduleTask(Task8);
			
			
			
		}
		
		
		__asm__ __volatile__("nop");
		PORTD ^= (1 << 0);
		TaskSetYield(tid, 787);

	}
	
	
	
	//Exit if we reached here
	while(!KillTask(tid));
}



/**
* \brief Task that blinks C:7 and schedules task 9 after 3 counts.
*/
static void Task8(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	uint8_t counter = 0;
	TaskSetYield(tid, 1000);
	
	while(1)
	{
		__asm__ __volatile__("nop");
		PORTC ^= (1 << 7);
		TaskSetYield(tid, 500);
			
		if(++counter > 3)
		{
				
			if(ScheduleTask(Task9) >= 0)
			{
				break;
			}
				
				
		}
	}
	
	PORTC &= ~(1 << 7);
	
	//Exit if we reached here
	while(!KillTask(tid));
}



/**
* \brief Task that blinks C:7 and schedules task 8 after 12 counts.
*/
static void Task9(void)
{
	TaskIndiceType_t tid = GetCurrentTaskID();
	uint8_t counter = 0;
	TaskSetYield(tid, 1000);
	
	while(1)
	{
		__asm__ __volatile__("nop");
		PORTC ^= (1 << 7);
		TaskSetYield(tid, 50);
			
		if(++counter > 12)
		{
				
			ScheduleTask(Task8);
			break;
		}
	}
	
	PORTC &= ~(1 << 7);
	while(!KillTask(tid));
}



/**
* \brief Task that forever reads ADC data and loads into the adc array
*/
static void AdcGetter(void)
{
	uint16_t currentAdcValue = 0;
	uint16_t currentAdcIndex = 0;
	
	TASK_RUN()
	{ 
		//Lock it down and...
		TASK_CRITICAL_SECTION_LOCK()
		{
			//Read the adc values without interruption
			m_AdcValues[currentAdcIndex] = SampleAdc(currentAdcIndex,2);
		}
		
		currentAdcIndex += 1;
		
		if(currentAdcIndex > 7)
		{
			currentAdcIndex = 0;
		}
		
		TaskSetYield(TaskRunID, 100);
	}

}



/**
* \brief Returns the ADC data from the ADC array, for easy reading during tasks
*/
static uint16_t TaskReadAdc(TaskIndiceType_t tid, uint8_t adcChannel)
{
	uint16_t adcValue = 0;
	
	//Lock it down and...
	TASK_CRITICAL_SECTION_LOCK()
	{
		adcValue = m_AdcValues[adcChannel];
	}
	
	
	return adcValue;
}