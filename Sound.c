//sound.c
// Runs on LM4F120 or TM4C123, 
// This routine calls the 4-bit DAC
// Use the Timer2 to request interrupts at a particular period.
// based on Daniel Valvano, Jonathan Valvano 

/*
Using Timer2
Configured on 32-bit mode
Configure on periodic mode, down-count
*/


#include "tm4c123gh6pm.h"
#include "Sound.h"


//Global variables
const unsigned char SineWave[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3};
unsigned int Index = 0;




// **************Sound_Tone*********************
// Change Timer2 interrupt to start sound output
// Input: interrupt period
//           Units of period are 12.5ns
//           Maximum is 2^24-1
//           Minimum is determined by length of ISR
// Output: none
void Sound_Tone(unsigned long period){
// this routine sets the RELOAD and starts SysTick
	NVIC_ST_RELOAD_R = period-1;						// reload value
	NVIC_ST_CTRL_R = 0x0007; 	
}