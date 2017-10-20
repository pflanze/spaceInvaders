//LED.c


#include "LED.h"
#include "..//tm4c123gh6pm.h"

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
