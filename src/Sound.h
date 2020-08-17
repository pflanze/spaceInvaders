#ifndef _SOUND_H
#define _SOUND_H


//Used to set the sound frequency
struct Sound {
	const unsigned int *data;
	const unsigned int size; //number of elements in data array
};

extern const struct Sound shoot;
extern const struct Sound smallExplosion;
extern const struct Sound ufoLowPitch;

//------------------------------------------------------------------------------------
	
// EXPORTED void shipFire(void);
// EXPORTED void DAC_Out(unsigned int data);
EXPORTED void Sound_Play(const struct Sound *ptSound);
EXPORTED void Sound_stop_all(const struct Sound *sp);


#endif /* _SOUND_H */
