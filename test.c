#include "trap.h"

#include "Buttons.h"
#include "GameEngine.h"
#include "Init.h"
#include "LED.h"
#include "Message.h"
#include "Nokia5110_font.h"
#include "Nokia5110.h"
#include "random.h"
#include "Sound.h"
#include "SpaceInvaders.h"
// #include "sprites.h"
#include "utils.h"


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void die_errno(const char* msg, const char* arg) {
	fprintf(stderr, "error: %s (%s): %s\n", msg, arg, strerror(errno));
	exit(1);
}


// Write screen buffer format from Nokia5110.c to file in PBM
// format. Take dimensions from Nokia5110.h
static void screen_write_pbm(const char* screen, const char* basepath) {
	int w= SCREENW;
	int h= SCREENH;

	int pathsiz= strlen(basepath)+5;
	char* path= malloc(pathsiz);
	snprintf(path, pathsiz, "%s.pbm", basepath);
	
	FILE *fh= fopen(path, "w");
	if (!fh) die_errno("open", path);

#define PRINTF(...) if(fprintf(fh, __VA_ARGS__) < 0) die_errno("print", path)

	PRINTF("P1\n");
	PRINTF("# CREATOR: SpaceInvaders test\n");
	PRINTF("%d %d\n", w, h);

	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int i= x + w*(y/8);
			char v= screen[i];
			PRINTF("%d ", ((v >> (y%8)) & 1) ? 1 : 0);
		}
		PRINTF("\n");
	}
	
#undef PRINTF
	
	if (fclose(fh)) die_errno("close", path);

	free(path);
}

static void screen_write_numbered(int i) {
	char basepath[10];
	snprintf(basepath, 10, "t/%i", i);
	screen_write_pbm(Screen, basepath);
}




static void game_step() {
	SysTick_Handler();
	main_update_LCD();
	GPIO_PORTE_DATA_R=0; // revert the push button to off
}

#define REPEAT(n, expr)  for(int i=0; i<n; i++) { expr; }

int main () {
	init_Hw();
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

	REPEAT(1, game_step());
	screen_write_numbered(1);
	
	GPIO_PORTE_DATA_R=1;
	REPEAT(10-1, game_step());
	screen_write_numbered(10);
	
	GPIO_PORTE_DATA_R=1;
	REPEAT(40-8-1, game_step());
	screen_write_numbered(40);

	// look at Nokia buffer.
	//trap();
	
	return 0;
}
