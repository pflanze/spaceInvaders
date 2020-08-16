//Mesagges to user

#include "Message.h"
#include "Nokia5110.h"

EXPORTED
void GameOverMessage(void){
	Nokia5110_Clear();
	Nokia5110_SetCursor(1, 1);
	Nokia5110_OutString("GAME OVER");
}

EXPORTED
void WinMessage(void){
	Nokia5110_Clear();
	Nokia5110_SetCursor(1, 1);
	Nokia5110_OutString("Lucky Shot");
	Nokia5110_SetCursor(1, 2);
	Nokia5110_OutString("Earthling");
}

EXPORTED
void RestartMessage(void){
	Nokia5110_Clear();
	Nokia5110_SetCursor(1, 1);
	Nokia5110_OutString("Press Fire");
	Nokia5110_SetCursor(1, 2);
	Nokia5110_OutString("to start");
}


EXPORTED
void InitMessage(void){
	Nokia5110_Clear();
	Nokia5110_SetCursor(1, 1);
	Nokia5110_OutString("We are");
	Nokia5110_SetCursor(1, 2);
	Nokia5110_OutString("under");
	Nokia5110_SetCursor(1, 3);
	Nokia5110_OutString("attack!");
}
