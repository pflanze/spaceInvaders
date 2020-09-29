//debug.h

//debugging code
#define DRAW_ENEMIES 1
#define DRAW_BONUSENEMY 1
#define GODMODE 1

#define AUDIO_1A	1
#define AUDIO_2A	1


#define PORTF1_audio_1A	0		//Enables/disables portF, Logic Analizer frequency test
#define PORTF1_audio_2A	0

#define PORTF1_systick	0		//Enables/disables portF, Logic Analizer frequency test


#ifdef DEBUG
#  include <stdio.h>
#  define WARN(fmt, ...)												\
	fprintf(stderr, "%s line %u: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#  define WARN(fmt, ...)
#endif
