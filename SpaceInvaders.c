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

#include "SpaceInvaders.h"
#ifndef TEST_WITHOUT_IO
#  include "..//tm4c123gh6pm.h"
#  include "TExaS.h"
#endif
#include "Nokia5110.h"
#include "Init.h"
#include "Buttons.h"
#include "GameEngine.h"
#include "random.h"
#include "Message.h"
#include "utils.h"
#include "Sound.h"
#include "debug.h"


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

gameStatus: End game@: FirstLast, EnemyLaserCollisions@MasterDraw
*/

//testing & preprocessing directives
#define IMESSAGE			0		//Enables/disables inittial message

//messages
#define SWAPDELAYMSG 10
#define SWAPDELAYMSG_2 SWAPDELAYMSG*2

volatile unsigned char SysTickFlag = 0;
volatile unsigned int gameOverFlag = STANDBY;
																//0: In game
																//1: Game Over (you loose)
																//2: Just Won
																//3: Game on standBy
volatile static unsigned char fireSound;
//function Prototypes
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

//********SysTick_Handler*****************
//Game sequence: STANDBY>INGAME>LOOSE|WIN
// inputs: none
// outputs: none
// assumes: na
void SysTick_Handler(void){			// runs at 30 Hz
	volatile static unsigned char clickCounter = 0;			//keeps track of clicks
	volatile static unsigned char multishot = 0;
	
#if PORTF1_systick
	GPIO_PORTF_DATA_R ^= 0x02;	//test only
#endif
	
	if(Pressfire_B1()){
		clickCounter++;
	}
	
	if(Pressfire_B2()){
		multishot = 1;
	}
	
	switch(gameOverFlag){
		case INGAME:{
			if(clickCounter){
				LaserInit_ship();
				Sound_Play(&shoot);
				clickCounter = 0;
			}	
			
			if(multishot){
				LaserInit_ship2();
				Sound_Play(&shoot);
				multishot = 0;
			}
#if DRAW_ENEMIES
			{static unsigned char EFcounter;
				EFcounter = (EFcounter+1)&FIREDEL;
				if(EFcounter >= FIREDEL){			//enemy shooting frequency
					EnemyLaserInit();
				}
			}
#endif
			
#if DRAW_ENEMYBONUS	
			enemyBonusCreate();
#endif	
			Collisions();
			{
				//update gameOverFlag only if different
				unsigned int status= getStatus();
				if(gameOverFlag != status){gameOverFlag = status;}			//it seems that there is need of a loop here
			}
			MoveObjects();				//game engine
			break;
		}
		case STANDBY:{
			{//sets defaults
				unsigned char rst = true;
				setStatus(gameOverFlag);
				if(rst){reset();rst=false;}
			}
			Player_Move();
			if(clickCounter == 1){
				LaserInit_ship();
				clickCounter = 0;
				Sound_Play(&shoot);
				gameOverFlag = INGAME;
				{//updates gameEngine with a new default value
					unsigned char done = true;
					if(done){setStatus(gameOverFlag);done = false;}
				}
			}
			break;
		}	
		default:{
			static char swapMessage = 0;
			Sound_stop_all(&ufoLowPitch);
			if(swapMessage < SWAPDELAYMSG){
				if(gameOverFlag == LOOSE){
					GameOverMessage();
#ifndef TEST_WITHOUT_IO
					GPIO_PORTB_DATA_R |= 0x20;
#endif
				}
				else{
					WinMessage();
#ifndef TEST_WITHOUT_IO
					GPIO_PORTB_DATA_R |= 0x10;
#endif
				}
			}
			else{
				RestartMessage();
				if(swapMessage > SWAPDELAYMSG_2){			//Swapmessage intermitent every 50 cycles
					swapMessage = 0;
				}
			}
			swapMessage++;
			if(clickCounter){
#if DRAW_ENEMIES
					EnemyInit();
					defaultValues();
#endif
#ifndef TEST_WITHOUT_IO
				Random_Init(NVIC_ST_CURRENT_R);
#endif
				ShipInit();
#if DRAW_ENEMYBONUS				
				BonusEnemy_Move(RESET);
#endif
				clickCounter = 0;
#ifndef TEST_WITHOUT_IO
				GPIO_PORTB_DATA_R &= ~0x30;
#endif
				gameOverFlag = STANDBY;
			}
		}
	}
  SysTickFlag = 1;
}
//********init_Hw*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
void init_Hw(void){
#ifndef TEST_WITHOUT_IO
	TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
#if PORTF1_systick || PORTF1_audio
	PortF_init();								//test only
#endif
  Nokia5110_Init();
#if AUDIO_1A
	Timer1A_Init();					//initialized @11kHz
#endif
#if AUDIO_2A
	Timer2A_Init();					//initialized @11kHz
#endif
	Systick_Init(2666666);			//initialized @30Hz
	ADC0_Init();
	Buttons_Init();
	DAC_Init();
	LED_Init();
	EnableInterrupts();
#endif
}
//********main*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
void main_update_LCD(void) {
	if((gameOverFlag == INGAME)||(gameOverFlag == STANDBY)){
		Draw(); // update the LCD
	}
}
//********main*****************
// Multiline description
// changes: variablesChanged
// Callers: 
// inputs: none
// outputs: none
// assumes: na
#ifndef TEST_WITHOUT_IO
int main(void){	
	init_Hw();											//call all initializing functions
	//Create initial message
#if IMESSAGE
	InitMessage();
#endif
	
#if DRAW_ENEMIES
	EnemyInit();
	defaultValues();
#endif
	ShipInit();
	Random_Init(1);
	
  while(1){
		while(SysTickFlag == 0){};
		main_update_LCD();
		SysTickFlag = 0;
	}
}
#endif

/*
ToDO:
	*Missing: enemyBonus explode
sound:
C:\WinSSDtemp\Home\desktop\KeepUpdating\Labware\Lab15_SpaceInvaders\Lab15Files\Sounds

*/
