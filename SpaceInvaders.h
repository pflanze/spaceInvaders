#ifndef _SPACEINVADERS_H
#define _SPACEINVADERS_H

#include "SpaceInvaders.h"
#include "object.h"


struct SpaceInvaders {
        struct ObjectInterface* vtable;
	struct GameEngine gameEngine;
	unsigned char SysTickFlag;
	unsigned int gameOverFlag; // XX why is this a copy of GameEngine_getStatus ?
	unsigned char clickCounter; // keeps track of clicks
	unsigned char multishot;
	unsigned char EFcounter;
	char swapMessage;
};

void SpaceInvaders_init(struct SpaceInvaders *this,
			unsigned int max_number_of_enemy_rows);
void SpaceInvaders_step(struct SpaceInvaders *this);
void SpaceInvaders_main_update_LCD(struct SpaceInvaders *this);


#endif
