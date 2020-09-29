#ifndef _SOUND_H
#define _SOUND_H

#include "utils.h"

struct Sound {
	const unsigned int *data;
	const unsigned int size; // number of elements in data array
};

#define NUM_SOUND_CHANNELS 2

struct SoundChannel {
#ifdef DEBUG
	const struct ObjectInterface *vtable;
#endif
	const struct Sound *playing; // NULL = channel is unused
	unsigned int frameIndex;
	unsigned int sampleIndex;
	unsigned int frame;
};

EXPORTED void SoundChannel_init(struct SoundChannel *this);


struct SoundPlayer {
#ifdef DEBUG
	const struct ObjectInterface *vtable;
#endif
	bool timerRunning;
	struct SoundChannel channel[NUM_SOUND_CHANNELS];
};

EXPORTED void SoundPlayer_init(struct SoundPlayer *this);

// find a free channel and play `sound` on it
EXPORTED void SoundPlayer_play(struct SoundPlayer *player,
							   const struct Sound *sound);
// stop any channel that is playing `sound`
EXPORTED void SoundPlayer_stop_all(struct SoundPlayer *player,
								   const struct Sound *sound);
// to be called for every output sample, usually from interrupt handler
EXPORTED void SoundPlayer_step(struct SoundPlayer *this);


#endif /* _SOUND_H */
