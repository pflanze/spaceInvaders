//Buttons

#include "Buttons.h"

#ifndef TEST_WITHOUT_IO
#  include "..//tm4c123gh6pm.h"
#else
bool GPIO_PORTE_DATA_R = 0;
#endif

EXPORTED
bool Pressfire_B1(void){
	static bool Fire_Bool,Prev_Fire;
	
	if ((GPIO_PORTE_DATA_R&0x01) && (Prev_Fire == 0)) { // just pressed
		Fire_Bool = true;
	}
	else {
		Fire_Bool = false;
	}
	Prev_Fire = (bool)(GPIO_PORTE_DATA_R&0x01);
	return Fire_Bool;
}

EXPORTED
bool Pressfire_B2(void){
	static bool Fire_Bool_2,Prev_Fire_2;
	
	if ((GPIO_PORTE_DATA_R&0x02) && (Prev_Fire_2 == 0)) { // just pressed
		Fire_Bool_2 = true;
	}
	else {
			Fire_Bool_2 = false;
	}
	Prev_Fire_2 = (bool)(GPIO_PORTE_DATA_R&0x02);
	return Fire_Bool_2;
}


//The mask needs changing, not pointing to the right adress
//Merge both functions into one
