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


static void game_step() {
	SysTick_Handler();
	main_update_LCD();
}

#define REPEAT(n, expr)  for(int i=0; i<n; i++) { expr; }

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

	REPEAT(1, game_step());
	GPIO_PORTE_DATA_R=1;
	
	REPEAT(8, game_step());

	// look at Nokia buffer.
	trap();
	
	return 0;
}
