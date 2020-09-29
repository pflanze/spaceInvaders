//sound.c
// Runs on LM4F120 or TM4C123, 
// This routine calls the 4-bit DAC
// Use the Timer2 to request interrupts at a particular period.
// based on Daniel Valvano, Jonathan Valvano 

/*
Using Timer2
Configured on 32-bit mode
Configure on periodic mode, down-count
*/

/*
How sound are implemented
- gameSound: should start when gameFlag==INGAME, then stop on WIN/LOOSE and bonusEnemy
- bonusEnemy: music is a mix of gameSound and bonusEnemy together, it should start when bonusEnemy appears and terminate when he dies... 
	then the music should go back to normal
- ExplosionSound: it should start on alive==0 or jk==0, finished at the end of the sound

*/

#ifndef TEST_WITHOUT_IO
#  include "tm4c123gh6pm.h"
#endif
#include "Sound.h"
#include "debug.h"
#include "utils.h"
#include "IO.h"



//-----------------Handlers----------------------------------------------

struct SoundState {
	const unsigned int timerN;
	const struct Sound *sound;
	unsigned int average;
	unsigned int frameIndex;
	unsigned int sampleIndex;
	unsigned int frame;
	unsigned int currentSample;
};
/*
write function that stop playing
create struct to pass parameters as pointers (reset parameters)
	which sound
	which timer
	average
	
a sound player 
*/

#if AUDIO_1A
static struct SoundState soundState1 = {1};
#endif

#if AUDIO_2A
static struct SoundState soundState2 = {2};
#endif


//********Timer1A_Handler*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
#if AUDIO_1A

//static unsigned int frameIndex_1A = 0, sampleIndex_1A = 0, currentSample_1A = 0, frame_1A = 0;

unsigned int average;

#ifndef TEST_WITHOUT_IO
static
void outputSounds() {
	average = (soundState1.currentSample + soundState2.currentSample) / 2;
	GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R &~ 0x0F) | average;
}
#endif
//********functionName*****************
// Multiline description
// changes: variablesChanged
// Callers: 
// inputs: none
// outputs: none
// assumes: na
//Used by: Function name
static
void soundStateInit(struct SoundState *soundState) {
	soundState->frameIndex = 0;
	soundState->sampleIndex = 0;
	soundState->currentSample = 0;
	soundState->frame = 0;
}
//********functionName*****************
// Multiline description
// changes: variablesChanged
// Callers: 
// inputs: none
// outputs: none
// assumes: na
//Used by: Function name
#ifndef TEST_WITHOUT_IO
static
void soundUpdate(struct SoundState *soundState) {
	
	if(soundState->sampleIndex == 8){
		soundState->frameIndex++;
		soundState->sampleIndex = 0;
	}

	if(soundState->sampleIndex == 0){
		soundState->frame = soundState->sound->data[soundState->frameIndex];
	}
	
	soundState->currentSample = (soundState->frame&0xF0000000)>>28;	
	
	outputSounds();
	
	soundState->sampleIndex++;
	soundState->frame <<= 4;
	
	if(soundState->frameIndex >= soundState->sound->size){
		switch(soundState->timerN){
			case 1:
					Timer1A_Stop();
					break;
			case 2:
					Timer2A_Stop();
					break;
		};
		soundStateInit(soundState);
	}
}
#endif

//********functionName*****************
// Multiline description
// changes: variablesChanged
// Callers: 
// inputs: none
// outputs: none
// assumes: na
//Used by: Function name
#ifndef TEST_WITHOUT_IO
EXPORTED /* it's not in Sound.h, though, nor referenced anywhere, ? */
void Timer1A_Handler(void) {
	TIMER1_ICR_R = 0x01;   // acknowledge timer1A timeout
	soundUpdate(&soundState1);
}
#endif

#endif
//********Timer2B_Handler*****************
//Multiline description
// inputs: none
// outputs: none
// assumes: na
//#if AUDIO_2A

#ifndef TEST_WITHOUT_IO
EXPORTED /* it's not in Sound.h, though, nor referenced anywhere, ? */
void Timer2A_Handler(void) {
	TIMER2_ICR_R = 0x01;   // acknowledge timer1A timeout
	soundUpdate(&soundState2);
}
#endif
//#endif

//------------------------------------------------------------------
//********functionName*****************
// Multiline description
// changes: variablesChanged
// Callers: 
// inputs: *ptArraySound
// outputs: none
// assumes: na
EXPORTED
void Sound_Play(const struct Sound *ptSound) {
#ifndef TEST_WITHOUT_IO
	volatile unsigned char timer1 = TIMER1_CTL_R&0x01;
	if (timer1 == 0 ) {
		soundState1.sound = ptSound;
		Timer1A_Start();
	}
#if AUDIO_2A	
	else {
		soundState2.sound = ptSound;
		Timer2A_Start();
	}
#endif
#endif
}


EXPORTED
void Sound_stop_all(const struct Sound *sp) {
	if(soundState1.sound == sp){
		Timer1A_Stop();
		soundStateInit(&soundState1);
	}
	
	if(soundState2.sound == sp){
		Timer2A_Stop();
		soundStateInit(&soundState2);
	}
}

/*
Modifications:
Precalculate ArraySize (sizeof) during compilation (using define)

BonusShip music should stop after explodes>>>> struct Sound should have a field to keep track of whick timers is in use for that sound



*/


