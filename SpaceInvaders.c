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

//testing & preprocessing directives
#define IMESSAGE			0		//Enables/disables inittial message


//messages
#define SWAPDELAYMSG 10
#define SWAPDELAYMSG_2 SWAPDELAYMSG*2

#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"
#include "Init.h"
#include "Buttons.h"
#include "GameEngine.h"
#include "Random.h"
#include "Message.h"

//Global variables
unsigned long TimerCount;
unsigned long Semaphore;
unsigned char SysTickFlag=0;
volatile unsigned char gameOverFlag = STANDBY;		
																//0: In game
																//1: Game Over (you loose)
																//2: Just Won
																//3: Game on standBy
//function Prototypes
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
//static void GameReset(void);	//resets the game


//********Timer2A_Handler*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
#if AUDIO
void Timer2A_Handler(void){ 
  TIMER2_ICR_R = 0x00000001;   // acknowledge timer2A timeout
	//GPIO_PORTF_DATA_R ^= 0x02;
	TimerCount++;
  Semaphore = 1; // trigger
}
#endif
//********SysTick_Handler*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
void SysTick_Handler(void){			// runs at 30 Hz
	volatile static unsigned clickCounter = 0;			//keeps track of clicks
	#if DRAW_ENEMIES
		static unsigned char EFcounter = 0;
	#endif
	
	#if PORTF1
		GPIO_PORTF_DATA_R ^= 0x02;	//test only
	#endif
	
	if(Pressfire_B1()){
		clickCounter++;
	}
	
	switch(gameOverFlag){
		case INGAME:
			if(clickCounter){
				LaserInit_ship();
				//Fire1_Sound();
				clickCounter = 0;
			}	
			#if DRAW_ENEMIES
			EFcounter++;
			if(EFcounter > 6){			//enemy shooting frequency
				EnemyLaserInit();
				EFcounter = 0;
			}
			#endif
			
			#if DRAW_ENEMYBONUS	
				enemyBonusCreate();
			#endif	
			CheckingCollisions();
			MoveObjects();				//game engine
			break;
		case STANDBY:
			Player_Move();
			if(clickCounter == 1){
				LaserInit_ship();
				clickCounter = 0;
				gameOverFlag = INGAME;
			}
			break;
		default:{
			static char swapMessage = 0;
			if(swapMessage < SWAPDELAYMSG){
				if(gameOverFlag == LOOSE){
					GameOverMessage();
				}
				else{
					WinMessage();
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
				#endif
				Random_Init(1);
				ShipInit();
				defaultValues();
				clickCounter = 0;
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
	TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
	
	#if PORTF1
		PortF_init();								//test only
	#endif
	
  
  Nokia5110_Init();
	
	#if AUDIO
		Timer2_Init(7272);					//initialized @11kHz
	#endif
	
	Systick_Init(2666666);			//initialized @30Hz
	
	ADC0_Init();
	Buttons_Init();
	DAC_Init();
	LED_Init();
	EnableInterrupts();
}
//********main*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
int main(void){	
	init_Hw();											//call all initializing functions
	//Create initial message
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

  while(1){
	 while(SysTickFlag == 0){};
		if((gameOverFlag == INGAME)||(gameOverFlag == STANDBY)){
			Draw(); // update the LCD
		}
    SysTickFlag = 0;
	}
}
