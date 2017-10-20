//Buttons

#include "..//tm4c123gh6pm.h"
#include "Buttons.h"

unsigned char Pressfire_B1(void){
	 static unsigned char Fire_Bool,Prev_Fire;
	
   if((GPIO_PORTE_DATA_R&0x01) && (Prev_Fire == 0)){ // just pressed
		 Fire_Bool = 1;
	 }
	 else{
			Fire_Bool = 0;
	 }
	Prev_Fire = GPIO_PORTE_DATA_R&0x01;
	return Fire_Bool;
}


unsigned char Pressfire_B2(void){
	 static unsigned char Fire_Bool_2,Prev_Fire_2;
	
   if((GPIO_PORTE_DATA_R&0x01) && (Prev_Fire_2 == 0)){ // just pressed
		 Fire_Bool_2 = 1;
	 }
	 else{
			Fire_Bool_2 = 0;
	 }
	Prev_Fire_2 = GPIO_PORTE_DATA_R&0x01;
	return Fire_Bool_2;
}


//The mask needs changing, not pointing to the right adress
//Merge both functions into one
