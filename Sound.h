//sound.h


//Used to set the sound frequency
struct Sound {
	const unsigned int *data;
	const unsigned int size;
};

extern const struct Sound shoot;
extern const struct Sound smallExplosion;
extern const struct Sound ufoLowPitch;

//------------------------------------------------------------------------------------
	
void shipFire(void);
void DAC_Out(unsigned int data);
void Sound_Play(const struct Sound *ptSound);
void Sound_stop_all(const struct Sound *sp);

