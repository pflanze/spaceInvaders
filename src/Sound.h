#ifndef _SOUND_H
#define _SOUND_H


struct Sound {
	const unsigned int *data;
	const unsigned int size; // number of elements in data array
};

EXPORTED void Sound_Play(const struct Sound *ptSound);
EXPORTED void Sound_stop_all(const struct Sound *sp);


#endif /* _SOUND_H */
