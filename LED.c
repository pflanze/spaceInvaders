//LED.c


#include "LED.h"

#ifndef TEST_WITHOUT_IO
#  include "../tm4c123gh6pm.h"
#else
unsigned char GPIO_PORTB_DATA_R=0;
#endif

static unsigned char n;

void Red(void){
	unsigned char i;
	for(i=0;n<4;n++){
	 GPIO_PORTB_DATA_R ^= 0x10;
	}
}

void Green(void){
	unsigned char i;
	for(i=0;n<4;n++){
	 GPIO_PORTB_DATA_R ^= 0x20;
	}
}
