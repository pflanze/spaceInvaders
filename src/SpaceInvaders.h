#ifndef _SPACEINVADERS_H
#define _SPACEINVADERS_H

#include "SpaceInvaders.h"
#include "object.h"


struct SpaceInvaders {
#ifdef DEBUG
	struct ObjectInterface* vtable;
#endif
	struct GameEngine gameEngine;
	bool sysTickFlag;
	unsigned int gameOverFlag; // XX why is this a copy of GameEngine_getStatus ?
	unsigned char clickCounter; // keeps track of clicks
	bool multishot;
	unsigned char EFcounter;
	char swapMessage;
};

void SpaceInvaders_init(struct SpaceInvaders *this,
						unsigned int max_number_of_enemy_rows);
void SpaceInvaders_step(struct SpaceInvaders *this);
void SpaceInvaders_main_update_LCD(struct SpaceInvaders *this);


#endif