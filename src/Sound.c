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



#ifdef DEBUG

static
void Sound_pp(struct Sound *this, FILE *out) {
	PP_PRINTF("&%s", this->symbolName);
}

static
void _Sound_pp(void *this, FILE *out) { Sound_pp(this, out); }

const struct ObjectInterface Sound_ObjectInterface = {
	.pp = &_Sound_pp
};


static
void SoundChannel_pp(struct SoundChannel *this, FILE *out) {
	PP_PRINTF("(struct SoundChannel) {");
	PP_PRINTF(" .playing = ");
	V(pp, this->playing, out);
	PP_PRINTF(", .frameIndex = %u", this->frameIndex);
	PP_PRINTF(", .sampleIndex = %u", this->sampleIndex);
	PP_PRINTF(", .frame = 0x%08x", this->frame);
	PP_PRINTF(" }");
}

static
void _SoundChannel_pp(void *this, FILE *out) { SoundChannel_pp(this, out); }

const struct ObjectInterface SoundChannel_ObjectInterface = {
	.pp = &_SoundChannel_pp
};

#endif

EXPORTED
void SoundChannel_init(struct SoundChannel *this) {
	*this = (struct SoundChannel){
		.frameIndex = 0,
		.sampleIndex = 0,
		.frame = 0,
	},
#ifdef DEBUG
	this->vtable = &SoundChannel_ObjectInterface;
#endif
}


#ifdef DEBUG

static
void SoundPlayer_pp(struct SoundPlayer *this, FILE *out) {
	PP_PRINTF("(struct SoundPlayer) {");
	PP_PRINTF(" .timerRunning = %s", bool_show(this->timerRunning));
	PP_PRINTF(", .channel = {");
	for (int i=0; i<NUM_SOUND_CHANNELS; i++) {
		if (i) {
			PP_PRINTF(", ");
		}
		V(pp, &this->channel[i], out);
	}
	PP_PRINTF(" }");
	PP_PRINTF(" }");
}

static
void _SoundPlayer_pp(void *this, FILE *out) { SoundPlayer_pp(this, out); }

const struct ObjectInterface SoundPlayer_ObjectInterface = {
	.pp = &_SoundPlayer_pp
};

#endif

EXPORTED
void SoundPlayer_init(struct SoundPlayer *this) {
	this->timerRunning = false;
	for (int i=0; i<NUM_SOUND_CHANNELS; i++) {
		SoundChannel_init(&this->channel[i]);
	}
#ifdef DEBUG
	this->vtable = &SoundPlayer_ObjectInterface;
#endif
}




#if AUDIO_1A

static
void writeSample(unsigned int sample) {
#ifndef TEST_WITHOUT_IO
	GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R &~ 0x0F) | sample;
#endif
}


EXPORTED
void SoundPlayer_step(struct SoundPlayer *this) {
	unsigned int sum = 0;
	unsigned int numchannels = 0;

	for (int i=0; i<NUM_SOUND_CHANNELS; i++) {
		struct SoundChannel *channel= &this->channel[i];
		if (channel->playing) {
	
			if (channel->sampleIndex == 8) {
				channel->frameIndex++;
				if (channel->frameIndex >= channel->playing->size) {
					SoundChannel_init(channel);
					continue;
				}
				channel->sampleIndex = 0;
			}

			if (channel->sampleIndex == 0) {
				channel->frame =
					channel->playing->data[channel->frameIndex];
			} else {
				channel->frame <<= 4;
			}
			channel->sampleIndex++;

			unsigned int currentSample = (channel->frame&0xF0000000)>>28;
			sum += currentSample;

			numchannels++;
		}
	}

	if (numchannels == 0) {
		Timer1A_Stop();
		this->timerRunning = false;
	} else {
		unsigned int average = sum / numchannels; // XXX cheaper division
		writeSample(average);
	}
}


#ifndef TEST_WITHOUT_IO
EXPORTED /* it's not in Sound.h, though, nor referenced anywhere, ? */
void Timer1A_Handler(void) {
	TIMER1_ICR_R = 0x01;   // acknowledge timer1A timeout
	soundUpdate(&soundState1);
}
#endif

#endif

#ifndef TEST_WITHOUT_IO
EXPORTED /* it's not in Sound.h, though, nor referenced anywhere, ? */
void Timer2A_Handler(void) {
	TIMER2_ICR_R = 0x01;   // acknowledge timer1A timeout
	soundUpdate(&soundState2);
}
#endif
//#endif


EXPORTED
void SoundPlayer_play(struct SoundPlayer *this,
					  const struct Sound *sound) {
	// find a free channel
	for (int i=0; i<NUM_SOUND_CHANNELS; i++) {
		struct SoundChannel *channel= &this->channel[i];
		if (! channel->playing) {
			channel->playing = sound;
			// The other SoundChannel fields should already be properly set.

			if (! this->timerRunning) {
				this->timerRunning = true; // *before* starting it to avoid race
				Timer1A_Start();
			}
			return;
		}
	}
	// couldn't find a free channel
	// XXXWARN;
}


EXPORTED
void SoundPlayer_stop_all(struct SoundPlayer *this,
						  const struct Sound *sound) {
	Timer1A_Stop();
	SoundPlayer_init(this);
}

/*
Modifications:
Precalculate ArraySize (sizeof) during compilation (using define)

BonusShip music should stop after explodes>>>> struct Sound should have a field to keep track of whick timers is in use for that sound



*/


