//sound.h


//Used to set the sound frequency
struct Sound {
	const unsigned int *data;
	const unsigned int size;
};

extern const struct Sound shoot;
//------------------------------------------------------------------------------------
	
void shipFire(void);
void DAC_Out(unsigned int data);
void Sound_Play(const struct Sound *ptSound);

