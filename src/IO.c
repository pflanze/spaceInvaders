#include "IO.h"

#ifndef TEST_WITHOUT_IO
#  include "tm4c123gh6pm.h"
#endif


void Timer1A_Stop(void){
#ifndef TEST_WITHOUT_IO
	TIMER1_CTL_R &= ~0x00000001; // disable
#endif
}

void Timer1A_Start(void){
#ifndef TEST_WITHOUT_IO
	TIMER1_CTL_R |= 0x00000001;   // enable
#endif
}	

void Timer2A_Stop(void){
#ifndef TEST_WITHOUT_IO
	TIMER2_CTL_R &= ~0x00000001; // disable
#endif
}

void Timer2A_Start(void){
#ifndef TEST_WITHOUT_IO
	TIMER2_CTL_R |= 0x00000001;   // enable
#endif
}

