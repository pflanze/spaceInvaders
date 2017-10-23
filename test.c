#include "trap.h"

#include "Buttons.h"
#include "GameEngine.h"
#include "Init.h"
#include "LED.h"
#include "Message.h"
#include "Nokia5110_font.h"
#include "Nokia5110.h"
#include "random.h"
#include "Sound.h"
#include "SpaceInvaders.h"
// #include "sprites.h"
#include "utils.h"




int main () {
	init_Hw();
	#if IMESSAGE
		InitMessage();
	#endif
	//Initializing game
	#if DRAW_ENEMIES
		EnemyInit();
	#endif
	ShipInit();
	defaultValues();
	Random_Init(1);
	
	SysTick_Handler();
	GPIO_PORTE_DATA_R=1;
	SysTick_Handler();
	SysTick_Handler();
	SysTick_Handler();
	SysTick_Handler();
	SysTick_Handler();
	SysTick_Handler();
	SysTick_Handler();

	// look at Nokia buffer.
	trap();
	
	return 0;
}
