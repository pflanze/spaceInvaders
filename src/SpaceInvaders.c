// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the edX Lab 15
// In order for other students to play your game
// 1) You must leave the hardware configuration as defined
// 2) You must not add/remove any files from the project
// 3) You must add your code only this this C file
// I.e., if you wish to use code from sprite.c or sound.c, move that code in this file
// 4) It must compile with the 32k limit of the free Keil

// April 10, 2014
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#ifndef TEST_WITHOUT_IO
#  include "..//tm4c123gh6pm.h"
#  include "TExaS.h"
#  include "Init.h"
#endif
#include "Nokia5110.h"
#include "Buttons.h"
#include "GameEngine.h"
#include "random.h"
#include "Message.h"
#include "utils.h"
#include "sounds.h"
#include "debug.h"
#include "SpaceInvaders.h"

#ifdef DEBUG
#  include <stdio.h>
#  include "pp.h"
#endif


static
void SpaceInvaders_pp(struct SpaceInvaders *this, FILE *out) {
	PP_PRINTF("(struct SpaceInvaders) {");
	PP_PRINTF(" .gameEngine = "); V(pp, &this->gameEngine, out);
	PP_PRINTF(", .sysTickFlag = %s", bool_show(this->sysTickFlag));
	PP_PRINTF(", .clickCounter = %u", this->clickCounter);
	PP_PRINTF(", .multishot = %s", bool_show(this->multishot));
	PP_PRINTF(", .EFcounter = %u", this->EFcounter);
	PP_PRINTF(", .swapMessage = %i", this->swapMessage);
	PP_PRINTF(" }");
}


static
void _SpaceInvaders_pp(void *this, FILE *out) { SpaceInvaders_pp(this, out); }

const struct ObjectInterface SpaceInvaders_ObjectInterface = {
	.pp = &_SpaceInvaders_pp
};


/*	Required Hardware I/O connections
Slide pot pin 1 connected to ground
Slide pot pin 2 connected to PE2/AIN1
Slide pot pin 3 connected to +3.3V 
fire button connected to PE0
special weapon fire button connected to PE1
8*R resistor DAC bit 0 on PB0 (least significant bit)
4*R resistor DAC bit 1 on PB1
2*R resistor DAC bit 2 on PB2
1*R resistor DAC bit 3 on PB3 (most significant bit)
LED on PB4
LED on PB5
*/


/*	Blue Nokia 5110
---------------
Signal        (Nokia 5110) LaunchPad pin
Reset         (RST, pin 1) connected to PA7
SSI0Fss       (CE,  pin 2) connected to PA3
Data/Command  (DC,  pin 3) connected to PA6
SSI0Tx        (Din, pin 4) connected to PA5
SSI0Clk       (Clk, pin 5) connected to PA2
3.3V          (Vcc, pin 6) power
back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
Ground        (Gnd, pin 8) ground
*/


/*	Red SparkFun Nokia 5110 (LCD-10168)
-----------------------------------
Signal        (Nokia 5110) LaunchPad pin
3.3V          (VCC, pin 1) power
Ground        (GND, pin 2) ground
SSI0Fss       (SCE, pin 3) connected to PA3
Reset         (RST, pin 4) connected to PA7
Data/Command  (D/C, pin 5) connected to PA6
SSI0Tx        (DN,  pin 6) connected to PA5
SSI0Clk       (SCLK, pin 7) connected to PA2
back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total
*/

/*
Notes:
Handlers:
Systick: @30hz: Bottons, ADC, Video calculations
Timer2: @44100hz: Sound

gameStatus: End game@: FirstLast, EnemyLaserCollisions@Actor_masterDraw
*/

//testing & preprocessing directives
#define IMESSAGE    0  // Enables/disables initial message

//messages
#define SWAPDELAYMSG 10
#define SWAPDELAYMSG_2 SWAPDELAYMSG*2


//function Prototypes
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts


//Game sequence: STANDBY>INGAME>LOOSE|WIN
// To be run from systick handler at 30 Hz
EXPORTED
void SpaceInvaders_step(struct SpaceInvaders *this) {
	random_mixin(ADC0_SSFIFO3_R);
	
#if PORTF1_systick
	GPIO_PORTF_DATA_R ^= 0x02;	//test only
#endif
	
	if(Pressfire_B1()){
		this->clickCounter++;
		random_mixin(101);
	}
	
	if(Pressfire_B2()){
		this->multishot = true;
		random_mixin(102);
	}
	
	switch(this->gameOverFlag){
		case INGAME:{
			if (this->clickCounter) {
				GameEngine_shipLasersCreation(&this->gameEngine, false);
				SoundPlayer_play(&this->gameEngine.soundPlayer, &shoot);
				this->clickCounter = 0;
			}	
			
			if (this->multishot) {
				GameEngine_laserInit_ship2(&this->gameEngine);
				SoundPlayer_play(&this->gameEngine.soundPlayer, &shoot);
				this->multishot = false;
			}
#if DRAW_ENEMIES
			{
				this->EFcounter = (this->EFcounter + 1) & FIREDEL;
				if (this->EFcounter >= FIREDEL) {
					// enemy shooting frequency
					GameEngine_enemyLasersCreation(&this->gameEngine, false);
				}
			}
#endif
			
#if DRAW_BONUSENEMY	
			GameEngine_bonusEnemyCreate(&this->gameEngine);
#endif	
			GameEngine_collisions(&this->gameEngine);
			{
				//update gameOverFlag only if different
				unsigned int status=
					GameEngine_getStatus(&this->gameEngine);
				if (this->gameOverFlag != status) {
					this->gameOverFlag = status;
				}
				// it seems that there is need of a loop here
			}
			GameEngine_moveObjects(&this->gameEngine);
			break;
		}
		case STANDBY:{
			{//sets defaults
				bool rst = true;
				GameEngine_setStatus(&this->gameEngine,
									 this->gameOverFlag);
				if (rst) {
					GameEngine_reset(&this->gameEngine);
					rst=false;
				}
			}
			GameEngine_player_move(&this->gameEngine);
			if (this->clickCounter == 1) {
				GameEngine_shipLasersCreation(&this->gameEngine, false);
				this->clickCounter = 0;
				SoundPlayer_play(&this->gameEngine.soundPlayer, &shoot);
				this->gameOverFlag = INGAME;
				{//updates gameEngine with a new default value
					bool done = true;
					if (done) {
						GameEngine_setStatus(&this->gameEngine,
											 this->gameOverFlag);
						done = false;
					}
				}
			}
			break;
		}	
		default: {
			SoundPlayer_stop_all(&this->gameEngine.soundPlayer, &ufoLowPitch);
			if (this->swapMessage < SWAPDELAYMSG) {
				if (this->gameOverFlag == LOOSE) {
					GameOverMessage();
#ifndef TEST_WITHOUT_IO
					GPIO_PORTB_DATA_R |= 0x20;
#endif
				}
				else {
					WinMessage();
#ifndef TEST_WITHOUT_IO
					GPIO_PORTB_DATA_R |= 0x10;
#endif
				}
			}
			else {
				RestartMessage();
				if (this->swapMessage > SWAPDELAYMSG_2) {
					// Swapmessage intermittent every 50 cycles
					this->swapMessage = 0;
				}
			}
			this->swapMessage++;
			if (this->clickCounter) {
#if DRAW_ENEMIES
				GameEngine_enemyInit(&this->gameEngine);
				GameEngine_defaultValues(&this->gameEngine);
#endif
				GameEngine_shipInit(&this->gameEngine);
#if DRAW_BONUSENEMY				
				GameEngine_bonusEnemy_move_reset(&this->gameEngine);
#endif
				this->clickCounter = 0;
#ifndef TEST_WITHOUT_IO
				GPIO_PORTB_DATA_R &= ~0x30;
#endif
				this->gameOverFlag = STANDBY;
			}
		}
	}
	this->sysTickFlag = true;
}

static
void init_Hw(void) {
#ifndef TEST_WITHOUT_IO
	TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
#if PORTF1_systick || PORTF1_audio
	PortF_init();  // test only
#endif
  Nokia5110_Init();
#if AUDIO_1A
	Timer1A_Init(); // initialized @11kHz
#endif
#if AUDIO_2A
	Timer2A_Init(); // initialized @11kHz
#endif
	Systick_Init(2666666); // initialized @30Hz
	ADC0_Init();
	Buttons_Init();
	DAC_Init();
	LED_Init();
	EnableInterrupts();
#endif
}


EXPORTED
void SpaceInvaders_init(struct SpaceInvaders *this,
						unsigned int max_number_of_enemy_rows) {
    this->vtable = &SpaceInvaders_ObjectInterface;
	this->sysTickFlag= false;
	this->gameOverFlag = STANDBY;
	this->clickCounter = 0;
	this->multishot = false;
	this->EFcounter = 0;
	this->swapMessage = 0;
	GameEngine_init(&this->gameEngine, max_number_of_enemy_rows);
	init_Hw(); // call all initializing functions; XX could this
			   // call be moved outside SpaceInvaders_init?
	//Create initial message
#if IMESSAGE
	InitMessage();
#endif
}

EXPORTED
void SpaceInvaders_main_update_LCD(struct SpaceInvaders *this) {
	if ((this->gameOverFlag == INGAME) || (this->gameOverFlag == STANDBY)) {
		GameEngine_draw(&this->gameEngine); // update the LCD
	}
}


#ifndef TEST_WITHOUT_IO

static struct SpaceInvaders game;

EXPORTED
void SysTick_Handler(void) {
	SpaceInvaders_step(&game);
}

int main(void) {
	random_init(1);
	SpaceInvaders_init(&game, 2 /* max_number_of_enemy_rows */);
	
	while (1) {
		while (sysTickFlag == false) {};
		SpaceInvaders_main_update_LCD(&game);
		sysTickFlag = false;
	}
}
#endif

/*
ToDO:
	*Missing: bonusEnemy explode
sound:
C:\WinSSDtemp\Home\desktop\KeepUpdating\Labware\Lab15_SpaceInvaders\Lab15Files\Sounds

*/
