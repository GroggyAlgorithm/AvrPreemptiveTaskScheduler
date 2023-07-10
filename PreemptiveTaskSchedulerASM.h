/**
 * \file PreemptiveTaskSchedulerASM.h
 * \author: Tim Robbins
 * \brief Assembly helper file for preemptive task scheduling and concurrent functionality. \n
 */ 
#ifndef __PREEMPTIVETASKSCHEDULERASM_H___
#define __PREEMPTIVETASKSCHEDULERASM_H___	1




/*
	ASM Constants
*/
//Program counter high/low
__asm__(".equ CONTEXT_OFFSET_PC_L, 33 \n\t");
__asm__(".equ CONTEXT_OFFSET_PC_H, 34 \n\t");



//Stack pointer high/low
__asm__(".equ CONTEXT_OFFSET_SP_L, 35 \n\t");
__asm__(".equ CONTEXT_OFFSET_SP_H, 36 \n\t");



//Register back offset
__asm__(".equ CONTEXT_OFFSET_R26,  9 \n\t");


//----------------------------------------------------------------------------------------------------




//ASM Macros and NAKED ASM FUNCTIONS------------------------------------------------------------------

///Command to Load a global ptr into Z
#define TaskSchedulerLoadGlobalZPtr(__taskContext) "lds ZL, " #__taskContext " \n\t" "lds ZH, " #__taskContext "+1 \n\t"


///Saves program context position. See naked function for comments.
#define _ASM_SAVE_CONTEXT(_z_load_cmds) \
asm volatile(  \
	"push r30 \n\t" \
	"push r31 \n\t" \
	"in r30, __SREG__ \n\t" \
	"cli \n\t" \
	"push r0 \n\t" \
	"push r30 \n\t" \
	_z_load_cmds \
	"pop r0 \n\t"  \
	"st Z+, r0 \n\t" \
	"pop r0 \n\t" \
	"st z+, r0 \n\t" \
	"st z+, r1 \n\t" \
	"st z+, r2 \n\t" \
	"st z+, r3 \n\t" \
	"st z+, r4 \n\t" \
	"st z+, r5 \n\t" \
	"st z+, r6 \n\t" \
	"st z+, r7 \n\t" \
	"st z+, r8 \n\t" \
	"st z+, r9 \n\t" \
	"st z+, r10 \n\t" \
	"st z+, r11 \n\t" \
	"st z+, r12 \n\t" \
	"st z+, r13 \n\t" \
	"st z+, r14 \n\t" \
	"st z+, r15 \n\t" \
	"st z+, r16 \n\t" \
	"st z+, r17 \n\t" \
	"st z+, r18 \n\t" \
	"st z+, r19 \n\t" \
	"st z+, r20 \n\t" \
	"st z+, r21 \n\t" \
	"st z+, r22 \n\t" \
	"st z+, r23 \n\t" \
	"st z+, r24 \n\t" \
	"st z+, r25 \n\t" \
	"st z+, r26 \n\t" \
	"st z+, r27 \n\t" \
	"st z+, r28 \n\t" \
	"st z+, r29 \n\t" \
	"mov r28, r30 \n\t" \
	"mov r29, r31 \n\t" \
	"pop r31 \n\t" \
	"pop r30 \n\t" \
	"st y+, r30 \n\t" \
	"st y+, r31 \n\t" \
	"pop r30 \n\t" \
	"pop r31 \n\t"  \
	"st y+, r31 \n\t" \
	"st y+, r30 \n\t" \
	"in r26, __SP_L__ \n\t" \
	"in r27, __SP_H__ \n\t" \
	"st y+, r26 \n\t"       \
	"st y, r27  \n\t" \
	"push r31 \n\t" \
	"push r30 \n\t" \
	"mov r30, r28 \n\t"    \
	"mov r31, r29 \n\t"                               \
	"in r28, __SREG__ \n\t" \
	"sbiw r30, CONTEXT_OFFSET_R26 \n\t" \
	"out __SREG__, r28 \n\t"   \
	"ld r26, Z+ \n\t"                                                  \
	"ld r27, Z+ \n\t"                                                  \
	"ld r28, Z+ \n\t"                                                  \
	"ld r29, Z+ \n\t"                   \
	"push r28 \n\t"                                                    \
	"push r29 \n\t"        \
	"mov r28, r30 \n\t"                                                \
	"mov r29, r31 \n\t"                                                \
	"ld r30, Y+ \n\t"                                                  \
	"ld r31, Y  \n\t" \
	"pop r29 \n\t"                                                     \
	"pop r28 \n\t"::)



///Restores program context position. See naked function for comments.
#define _ASM_RESTORE_CONTEXT(_z_load_cmds) \
asm volatile( \
	_z_load_cmds \
	"adiw r30, CONTEXT_OFFSET_SP_H \n\t" \
	"cli \n\t" \
	"ld r0, Z \n\t" \
	"out __SP_H__, r0 \n\t" \
	"ld r0, -Z \n\t" \
	"out __SP_L__, r0 \n\t" \
	"ld r1, -Z \n\t" \
	"ld r0, -Z \n\t" \
	"push r0 \n\t" \
	"push r1 \n\t" \
	"mov r28, r30 \n\t"                                                 \
	"mov r29, r31 \n\t"                                                 \
	"ld r31, -Y \n\t"                                                   \
	"ld r30, -Y \n\t"                                                   \
	"push r31 \n\t"                                                     \
	"push r30 \n\t"                                                     \
	"mov r30, r28 \n\t"                                                 \
	"mov r31, r29 \n\t"                                                 \
	"ld r29, -Z \n\t" \
	"ld r28, -Z \n\t" \
	"ld r27, -Z \n\t" \
	"ld r26, -Z \n\t" \
	"ld r25, -Z \n\t" \
	"ld r24, -Z \n\t" \
	"ld r23, -Z \n\t" \
	"ld r22, -Z \n\t" \
	"ld r21, -Z \n\t" \
	"ld r20, -Z \n\t" \
	"ld r19, -Z \n\t" \
	"ld r18, -Z \n\t" \
	"ld r17, -Z \n\t" \
	"ld r16, -Z \n\t" \
	"ld r15, -Z \n\t" \
	"ld r14, -Z \n\t" \
	"ld r13, -Z \n\t" \
	"ld r12, -Z \n\t" \
	"ld r11, -Z \n\t" \
	"ld r10, -Z \n\t" \
	"ld r9, -Z \n\t" \
	"ld r8, -Z \n\t" \
	"ld r7, -Z \n\t" \
	"ld r6, -Z \n\t" \
	"ld r5, -Z \n\t" \
	"ld r4, -Z \n\t" \
	"ld r3, -Z \n\t" \
	"ld r2, -Z \n\t" \
	"ld r1, -Z \n\t" \
	"ld r0, -Z \n\t" \
	"push r0 \n\t" \
	"ld r0, -Z \n\t" \
	"out __SREG__, r0 \n\t" \
	"pop r0 \n\t" \
	"pop r30 \n\t" \
	"pop r31 \n\t"::)





///Restores a global pointers context. See naked functions for comments.
#define ASM_SAVE_GLOBAL_PTR_CONTEXT(_ptr) \
_ASM_SAVE_CONTEXT( "lds ZL, " #_ptr " \n\t" "lds ZH, " #_ptr "+1 \n\t")



///Restores a global pointers context. See naked functions for comments.
#define ASM_RESTORE_GLOBAL_PTR_CONTEXT(_ptr) \
_ASM_RESTORE_CONTEXT( "lds ZL, " #_ptr " \n\t" "lds ZH, " #_ptr " + 1 \n\t")



/**
 * \brief Saves program context into the passed Context
 * \param taskContext The context structure to save to
 */
__attribute__((naked)) static void SaveContext(volatile TaskContext_t *taskContext)
{
	
	asm volatile
	(
		//Push the Z registers onto the stack
		"push r30 \n\t"
		"push r31 \n\t"
		
		//Save SREG
		"in r30, __SREG__ \n\t"
		
		//"\n" presave_code "\n\t"
		
		//Disable Interrupts
		"cli \n\t"
		
		//Save r0 temp register
		"push r0 \n\t"
		
		//Push SREG value onto the stack
		"push r30 \n\t"
		
		//Move data from the Z address
		"mov r30, r24 \n\t"
		"mov r31, r25 \n\t"
		 
		//Pop from the stack into R0 register
		"pop r0 \n\t" 
		
		/*
		
			Task context formatting:
			
			1st: SREG value
			2nd: Stored register values
			3rd: Program counter
			4th: Stack Pointer
		
		*/
		
		//Save SREG to our context structure 
		"st Z+, r0 \n\t"                      
		
		//Restore initial R0 value by popping the stack into R0.
		"pop r0 \n\t"
		 
		// Save general purpose register file values.
		"st z+, r0 \n\t"
		"st z+, r1 \n\t"
		"st z+, r2 \n\t"
		"st z+, r3 \n\t"
		"st z+, r4 \n\t"
		"st z+, r5 \n\t"
		"st z+, r6 \n\t"
		"st z+, r7 \n\t"
		"st z+, r8 \n\t"
		"st z+, r9 \n\t"
		"st z+, r10 \n\t"
		"st z+, r11 \n\t"
		"st z+, r12 \n\t"
		"st z+, r13 \n\t"
		"st z+, r14 \n\t"
		"st z+, r15 \n\t"
		"st z+, r16 \n\t"
		"st z+, r17 \n\t"
		"st z+, r18 \n\t"
		"st z+, r19 \n\t"
		"st z+, r20 \n\t"
		"st z+, r21 \n\t"
		"st z+, r22 \n\t"
		"st z+, r23 \n\t"
		"st z+, r24 \n\t"
		"st z+, r25 \n\t"
		"st z+, r26 \n\t"
		"st z+, r27 \n\t"
		"st z+, r28 \n\t"
		"st z+, r29 \n\t"
		
		//Move r30 and r31 into r28 and r29 (28 and 29 have already been stored in our struct)
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		
		//Pop from the stack into R31 and R30
		"pop r31 \n\t"
		"pop r30 \n\t"
		
		//Store r30 and r31 at y+
		"st y+, r30 \n\t"
		"st y+, r31 \n\t"
									
		//Pop what should now be the return address 
		"pop r30 \n\t" // high part
		"pop r31 \n\t" // low part 
		
		//and save at Y post increment
		"st y+, r31 \n\t"
		"st y+, r30 \n\t"
		
		//Store our stack pointer into r26 and r27
		"in r26, __SP_L__ \n\t"                                            
		"in r27, __SP_H__ \n\t"
		
		//Save the stack pointer into the structure.                                     
		"st y+, r26 \n\t"                                                  
		"st y, r27  \n\t"
													
		//Push the return address back at the top of the stack.      
		"push r31 \n\t" // low part                                      
		"push r30 \n\t" // high part  
		                                 
		//Context now saved
		 
		//But...registers 26, 27, 28, 29, 30, and 31 are now clobbered.
		
		//To provide generic usage, restore the values even if we don't need to.
		
		//Switch from Y pointer register to Z   
		"mov r30, r28 \n\t"   
		"mov r31, r29 \n\t"                              
		
		//Save our SREG value into R28
		"in r28, __SREG__ \n\t"
		
		//Go to the offset of R26 in our context structure          
		"sbiw r30, CONTEXT_OFFSET_R26 \n\t"
		
		//Restore our SREG value
		"out __SREG__, r28 \n\t"  
		                     
		//Load registers 26-29 from our data structure
		"ld r26, Z+ \n\t"                                                 
		"ld r27, Z+ \n\t"                                                 
		"ld r28, Z+ \n\t"                                                 
		"ld r29, Z+ \n\t"                  
									
		//Push R28, R29 (Y) on the stack to save
		"push r28 \n\t"                                                   
		"push r29 \n\t"       
										
		//Switch to our other index register (z to y) and read r30 and r31
		"mov r28, r30 \n\t"                                               
		"mov r29, r31 \n\t"                                               
		"ld r30, Y+ \n\t"                                                 
		"ld r31, Y  \n\t"                     
									
		//Restore R28, R29 (Y index) from the stack
		"pop r29 \n\t"                                                    
		"pop r28 \n\t"
		 
		:
		: 
		
	);
	
	
	__asm__ __volatile__ ("ret\n");
}



/**
 * \brief Saves program context into passed Context A and restores from passed Context B
 * \param taskContextA The context structure to save to
 * \param taskContextB The context structure to restore from
 */
__attribute__((naked)) static void SwapContext(volatile TaskContext_t *taskContextA, volatile TaskContext_t *taskContextB)
{

	/*
		Save Context
	*/
	asm volatile
	(
		//Push the Z registers onto the stack
		"push r30 \n\t"
		"push r31 \n\t"
		
		//Save SREG
		"in r30, __SREG__ \n\t"
		
		//"\n" presave_code "\n\t"
		
		//Disable Interrupts
		"cli \n\t"
		
		//Save r0 temp register
		"push r0 \n\t"
		
		//Push SREG value onto the stack
		"push r30 \n\t"

		//Move data from the Z address
		 "mov r30, r22 \n\t"
		 "mov r31, r23 \n\t"
		 
		//Pop from the stack into R0 register
		"pop r0 \n\t" 
		
		
		/*
		
		Task context formatting:
		
		1st: SREG value
		2nd: Stored register values
		3rd: Program counter
		4th: Stack Pointer
		
		*/
		
		//Save SREG to our context structure 
		"st Z+, r0 \n\t"                      
		
		//Restore initial R0 value by popping the stack into R0.
		"pop r0 \n\t"
		
		// Save general purpose register file values.
		"st z+, r0 \n\t"
		"st z+, r1 \n\t"
		"st z+, r2 \n\t"
		"st z+, r3 \n\t"
		"st z+, r4 \n\t"
		"st z+, r5 \n\t"
		"st z+, r6 \n\t"
		"st z+, r7 \n\t"
		"st z+, r8 \n\t"
		"st z+, r9 \n\t"
		"st z+, r10 \n\t"
		"st z+, r11 \n\t"
		"st z+, r12 \n\t"
		"st z+, r13 \n\t"
		"st z+, r14 \n\t"
		"st z+, r15 \n\t"
		"st z+, r16 \n\t"
		"st z+, r17 \n\t"
		"st z+, r18 \n\t"
		"st z+, r19 \n\t"
		"st z+, r20 \n\t"
		"st z+, r21 \n\t"
		"st z+, r22 \n\t"
		"st z+, r23 \n\t"
		"st z+, r24 \n\t"
		"st z+, r25 \n\t"
		"st z+, r26 \n\t"
		"st z+, r27 \n\t"
		"st z+, r28 \n\t"
		"st z+, r29 \n\t"
		
		//Move r30 and r31 into r28 and r29 (28 and 29 have already been stored in our struct)
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		
		//Pop from the stack into R31 and R30
		"pop r31 \n\t"
		"pop r30 \n\t"
		
		//Store r30 and r31 at y+
		"st y+, r30 \n\t"
		"st y+, r31 \n\t"

		//Pop what should now be the return address 
		"pop r30 \n\t" // high part
		"pop r31 \n\t" // low part 
		
		//and save at Y post increment
		"st y+, r31 \n\t"
		"st y+, r30 \n\t"
		
		//Store our stack pointer into r26 and r27
		"in r26, __SP_L__ \n\t"
		"in r27, __SP_H__ \n\t"
		
		//Save the stack pointer into the structure.
		"st y+, r26 \n\t"
		"st y, r27  \n\t"

		//Push the return address back at the top of the stack.
		"push r31 \n\t" // low part
		"push r30 \n\t" // high part

		//Context now saved
		
		//But...registers 26, 27, 28, 29, 30, and 31 are now clobbered.
		
		//To provide generic usage, restore the values even if we don't need to.
		
		//Switch from Y pointer register to Z   
		"mov r30, r28 \n\t"
		"mov r31, r29 \n\t"
		
		//Save our SREG value into R28
		"in r28, __SREG__ \n\t"
		
		//Go to the offset of R26 in our context structure          
		"sbiw r30, CONTEXT_OFFSET_R26 \n\t"
		
		//Restore our SREG value
		"out __SREG__, r28 \n\t" 
	
		//Load registers 26-29 from our data structure
		"ld r26, Z+ \n\t"
		"ld r27, Z+ \n\t"
		"ld r28, Z+ \n\t"
		"ld r29, Z+ \n\t"

		//Push R28, R29 (Y) on the stack to save
		"push r28 \n\t"
		"push r29 \n\t"

		//Switch to our other index register (z to y) and read r30 and r31
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		"ld r30, Y+ \n\t"
		"ld r31, Y  \n\t"

		//Restore R28, R29 (Y index) from the stack
		"pop r29 \n\t"
		"pop r28 \n\t"

		:
		:
		
	);
	
	/*
		Restore Context
	*/
	asm volatile
	(
		//Move data from the Z address
		 "mov r30, r24 \n\t"
		 "mov r31, r25 \n\t"
		//_LOAD_ADDRESS_Z                               

		//Go to the end of the context structure and start restoring it from there.
		"adiw r30, CONTEXT_OFFSET_SP_H \n\t"
		
		//Disable interrupts
		"cli \n\t"
		
		//Restore the saved stack pointer.
		"ld r0, Z \n\t"
		"out __SP_H__, r0 \n\t"
		"ld r0, -Z \n\t"
		"out __SP_L__, r0 \n\t"
		
		//Put the saved return address (PC) back on the top of the stack.
		"ld r1, -Z \n\t" //high part 
		"ld r0, -Z \n\t" //low part 
		
		"push r0 \n\t"
		"push r1 \n\t"
		
		//Temporarily switch pointer from Z index to Y index,
		//restore r31, r30 (Z) and put them on top of the stack.     
		"mov r28, r30 \n\t"
		"mov r29, r31 \n\t"
		"ld r31, -Y \n\t"
		"ld r30, -Y \n\t"
		"push r31 \n\t"
		"push r30 \n\t"
		
		//Switch back from Y index to Z index. 
		"mov r30, r28 \n\t"
		"mov r31, r29 \n\t"
		
		//Restore general purpose file registers.                   
		"ld r29, -Z \n\t"
		"ld r28, -Z \n\t"
		"ld r27, -Z \n\t"
		"ld r26, -Z \n\t"
		"ld r25, -Z \n\t"
		"ld r24, -Z \n\t"
		"ld r23, -Z \n\t"
		"ld r22, -Z \n\t"
		"ld r21, -Z \n\t"
		"ld r20, -Z \n\t"
		"ld r19, -Z \n\t"
		"ld r18, -Z \n\t"
		"ld r17, -Z \n\t"
		"ld r16, -Z \n\t"
		"ld r15, -Z \n\t"
		"ld r14, -Z \n\t"
		"ld r13, -Z \n\t"
		"ld r12, -Z \n\t"
		"ld r11, -Z \n\t"
		"ld r10, -Z \n\t"
		"ld r9, -Z \n\t"
		"ld r8, -Z \n\t"
		"ld r7, -Z \n\t"
		"ld r6, -Z \n\t"
		"ld r5, -Z \n\t"
		"ld r4, -Z \n\t"
		"ld r3, -Z \n\t"
		"ld r2, -Z \n\t"
		"ld r1, -Z \n\t"
		"ld r0, -Z \n\t"
		
		//Push R0 onto the stack to save
		"push r0 \n\t"
		
		//Restore SREG from our structure
		"ld r0, -Z \n\t"
		"out __SREG__, r0 \n\t"
		
		//Pop R0 back off the stack
		"pop r0 \n\t"
		
		//Restore r31, r30 (Z index) from the stack.
		"pop r30 \n\t"
		"pop r31 \n\t"
		
		
		//Enable Global Interrupts
		"sei \n\t"
		
		:
		:
	);
	
	//Return
	asm volatile("reti \n\t");
	//__asm__ __volatile__ ("ret\n");
}



/**
 * \brief Restores from passed Context
 * \param taskContext The context structure to restore from
 */
__attribute__((naked)) static void RestoreContext(volatile TaskContext_t *taskContext)
{
	
	/*
		Restore Context
	*/
	asm volatile
	(
		//Move data from the Z address
		"mov r30, r24 \n\t"
		"mov r31, r25 \n\t"

		//Go to the end of the context structure and start restoring it from there.
		"adiw r30, CONTEXT_OFFSET_SP_H \n\t"
		
		//Disable interrupts
		"cli \n\t"
		
		//Restore the saved stack pointer.
		"ld r0, Z \n\t"
		"out __SP_H__, r0 \n\t"
		"ld r0, -Z \n\t"
		"out __SP_L__, r0 \n\t"
		
		//Put the saved return address (PC) back on the top of the stack.
		"ld r1, -Z \n\t" //high part 
		"ld r0, -Z \n\t" //low part 
		
		"push r0 \n\t"
		"push r1 \n\t"
		
		//Temporarily switch pointer from Z to Y,                    
		//restore r31, r30 (Z) and put them on top of the stack.     
		"mov r28, r30 \n\t"                                                
		"mov r29, r31 \n\t"                                                
		"ld r31, -Y \n\t"                                                  
		"ld r30, -Y \n\t"                                                  
		"push r31 \n\t"                                                    
		"push r30 \n\t"                                                    
		
		//Switch back from Y to Z.                                   
		"mov r30, r28 \n\t"                                                
		"mov r31, r29 \n\t"                                                
		
		//Restore other general purpose registers.                   
		"ld r29, -Z \n\t"
		"ld r28, -Z \n\t"
		"ld r27, -Z \n\t"
		"ld r26, -Z \n\t"
		"ld r25, -Z \n\t"
		"ld r24, -Z \n\t"
		"ld r23, -Z \n\t"
		"ld r22, -Z \n\t"
		"ld r21, -Z \n\t"
		"ld r20, -Z \n\t"
		"ld r19, -Z \n\t"
		"ld r18, -Z \n\t"
		"ld r17, -Z \n\t"
		"ld r16, -Z \n\t"
		"ld r15, -Z \n\t"
		"ld r14, -Z \n\t"
		"ld r13, -Z \n\t"
		"ld r12, -Z \n\t"
		"ld r11, -Z \n\t"
		"ld r10, -Z \n\t"
		"ld r9, -Z \n\t"
		"ld r8, -Z \n\t"
		"ld r7, -Z \n\t"
		"ld r6, -Z \n\t"
		"ld r5, -Z \n\t"
		"ld r4, -Z \n\t"
		"ld r3, -Z \n\t"
		"ld r2, -Z \n\t"
		"ld r1, -Z \n\t"
		"ld r0, -Z \n\t"
		
		//Restore SREG
		"push r0 \n\t"
		"ld r0, -Z \n\t"
		"out __SREG__, r0 \n\t"
		"pop r0 \n\t"
		
		//Restore r31, r30 (Z) from the stack.
		"pop r30 \n\t"
		"pop r31 \n\t"
		

		//Enable Global Interrupts
		"sei \n\t"
		:
		:
	);
	
	//Return
	__asm__ __volatile__ ("ret\n");
}



//----------------------------------------------------------------------------------------------------



#endif /* PREEMPTIVETASKSCHEDULERASM_H_ */