#ifndef _SOUND_H
#define _SOUND_H

#include "utils.h"
#include "object.h"
#include "pp.h" //XX shouldn't that be part of object.h


struct Sound {
#ifdef DEBUG
	const struct ObjectInterface *vtable;
	const char *symbolName;
#endif
	const unsigned int *data;
	const unsigned int size; // number of elements in data array
};

#ifdef DEBUG
extern const struct ObjectInterface Sound_ObjectInterface;
#endif

#define DEFINE_SOUND_BEGIN(name)						\
	const unsigned int _##name [] =

#ifdef DEBUG
#define DEFINE_SOUND_END(name)							\
	;													\
	const struct Sound name = {							\
		.vtable = &Sound_ObjectInterface,				\
		.symbolName = #name,							\
		.data = _##name,								\
		.size = sizeof(_##name)/sizeof(_##name[0])		\
	}
#else
// copy-paste with .vtable and .symbolName removed
#define DEFINE_SOUND_END(name)							\
	;													\
	const struct Sound name = {							\
		.data = _##name,								\
		.size = sizeof(_##name)/sizeof(_##name[0])		\
	}
#endif

#define MAX_SAMPLE 15
#define SILENCE_SAMPLE 7

#define NUM_SOUND_CHANNELS 3

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
#ifdef DEBUG
	unsigned int currentSample;
#endif
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
