// #include "trap.h"

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
	snprintf(basepath, 10, "t/%04i", i);
	screen_write_pbm(Screen, basepath);
}



struct Game {
	int frame_number;
};

static void game_step(struct Game *game) {
	SysTick_Handler();
	main_update_LCD();
	GPIO_PORTE_DATA_R=0; // revert the push button to off
	game->frame_number++;
}

#define REPEAT(n, expr)  for(int i=0; i<n; i++) expr

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
	Random_Init(223412);

	struct Game game;
	game.frame_number=-1;

	game_step(&game);
	screen_write_numbered(game.frame_number);

	REPEAT(8,
	       {
		       GPIO_PORTE_DATA_R=1;
		       REPEAT(10,
			      {
				      game_step(&game);
				      screen_write_numbered(game.frame_number);
			      });
	       });

	/* verify that there are no differences to the committed
	   versions of the frames */

	// assume that MingW's shell can do redirection
	system("git status --porcelain t > t.out");

	const char* path= "t.out";
	FILE *fh= fopen(path, "r");
	if (!fh) die_errno("open", path);
	char buf[1024];
	int got= fread(buf, 1, 1014, fh);
	if (ferror(fh)) die_errno("fread", path);
	if (fclose(fh)) die_errno("close", path);

	if (got==0) {
		return 0;
	} else {
		printf("test failures:\n");
		system("cat t.out");
		return 1;
	}
}
