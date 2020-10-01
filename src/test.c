// #include "trap.h"

#include "Buttons.h"
#include "GameEngine.h"
#include "Message.h"
#include "Nokia5110_font.h"
#include "Nokia5110.h"
#include "random.h"
#include "Sound.h"
#include "SpaceInvaders.h"
// #include "sprites.h"
#include "utils.h"
#include "pp.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "perhaps_assert.h"
#include "stdlib_utils.h"


#define XSNPRINTF(str, n, ...) \
	assert(snprintf(str, n, __VA_ARGS__) < n)

static
void die_errno(const char *msg, const char *arg) {
	fprintf(stderr, "error: %s (%s): %s\n", msg, arg, strerror(errno));
	exit(1);
}

#if __CYGWIN__
#  define NL "\r\n"
#else 
#  define NL "\n"
#endif


// Write screen buffer format from Nokia5110.c to file in XPM
// format. Take dimensions from Nokia5110.h
static
void screen_write_xpm(const char *screen, const char *basepath) {
	int w= SCREENW;
	int h= SCREENH;

	int pathsiz= strlen(basepath)+5;
	char *path= xmalloc(pathsiz);
	snprintf(path, pathsiz, "%s.xpm", basepath);
	
	FILE *fh= fopen(path, "w");
	if (!fh) die_errno("open", path);

#define PRINTF(...) if(fprintf(fh, __VA_ARGS__) < 0) die_errno("print", path)

	PRINTF("%s",
	       "/* XPM */"NL
	       "static char *foo[] = {"NL);
	/* columns rows colors chars-per-pixel */
	PRINTF("\"%d %d %d %d\","NL, w, h, 2, 1);
	PRINTF("%s",
	       "\"X c black\","NL
	       "\"  c white\","NL);
	for (int y=0; y<h; y++) {
		PRINTF("\"");
		for (int x=0; x<w; x++) {
			int i= x + w*(y/8);
			char v= screen[i];
			PRINTF("%c", ((v >> (y%8)) & 1) ? 'X' : ' ');
		}
		PRINTF("\","NL);
	}
	PRINTF("};"NL);
	
#undef PRINTF
	
	if (fclose(fh)) die_errno("close", path);

	free(path);
}


struct Game {
	unsigned int max_number_of_enemy_rows;
	int frame_number;
	struct SpaceInvaders spaceInvaders;
};

static
void game_screen_write(struct Game *game) {
	char basepath[100];
	XSNPRINTF(basepath, 100, "out/%i-%04i",
			  game->max_number_of_enemy_rows, game->frame_number);
	screen_write_xpm(Screen, basepath);
}


#define REPEAT(n)  for(int i=0; i<n; i++) 


static
void game_step(struct Game *game, FILE *step_dump_fh, FILE *sound_dump_fh) {
	// one step in the game, except for the audio sample handler:
	PP_TO(&game->spaceInvaders, step_dump_fh);
	SpaceInvaders_step(&game->spaceInvaders);
	SpaceInvaders_main_update_LCD(&game->spaceInvaders);
	GPIO_PORTE_DATA_R=0; // revert the push button to off
	game->frame_number++;

	// audio sample handler:
	PP_TO(&game->spaceInvaders, sound_dump_fh);
	struct SoundPlayer *soundPlayer = &game->spaceInvaders.gameEngine.soundPlayer;
	REPEAT(1000) {
		if (soundPlayer->timerRunning) {
			PP_TO(soundPlayer, sound_dump_fh);
			SoundPlayer_step(soundPlayer);
		} else {
			fprintf(sound_dump_fh, ".timerRunning = false\n");
		}
	}
}


#define LET_XMALLOC(var, type) type *var = xmalloc(sizeof(*var))

#define DUMPFILE_PATHSIZ 100
struct DumpFile {
	char path[DUMPFILE_PATHSIZ];
	FILE *out;
};

static
struct DumpFile *DumpFile_xopen(unsigned int i, const char *name) {
	LET_XMALLOC(this, struct DumpFile);
	snprintf(this->path, DUMPFILE_PATHSIZ, "%i-%s.dump", i, name);
	this->out = fopen(this->path, "w");
	if (! this->out) {
		die_errno("open", this->path);
	}
	return this;
}

static
void DumpFile_xclose_and_free(struct DumpFile *this) {
	if (fclose(this->out) != 0) {
		die_errno("close", this->path);
	}
	free(this);
}

static
void test_run(unsigned int max_number_of_enemy_rows) {
	struct Game game;

	/*
	  for comparing test results with and without (it *should* have no effect):
	*/
	//memset(&game, 8, sizeof(game));

	struct DumpFile *stepDf = DumpFile_xopen(max_number_of_enemy_rows, "step");
	struct DumpFile *soundDf = DumpFile_xopen(max_number_of_enemy_rows, "sound");

	game.max_number_of_enemy_rows= max_number_of_enemy_rows;
	game.frame_number= -1;

	random_init(223412);

	SpaceInvaders_init(&game.spaceInvaders,
					   max_number_of_enemy_rows);
	ADC0_SSFIFO3_R= 0;

	game_step(&game, stepDf->out, soundDf->out);
	game_screen_write(&game);

	REPEAT(8) {
		GPIO_PORTE_DATA_R=1;
		REPEAT(10) {
			game_step(&game, stepDf->out, soundDf->out);
			game_screen_write(&game);
		}
	}

	ADC0_SSFIFO3_R= 2000;

	REPEAT(8) {
		GPIO_PORTE_DATA_R=1;
		REPEAT(10) {
			game_step(&game, stepDf->out, soundDf->out);
			game_screen_write(&game);
		}
	}

	DumpFile_xclose_and_free(soundDf);
	DumpFile_xclose_and_free(stepDf);
}


int main () {
	test_run(1);
	test_run(2);

	/* verify that there are no differences to the committed
	   versions of the frames */

	// assume that MingW's shell can do redirection
	system("git status --porcelain out > t.out");

	const char *path= "t.out";
	FILE *fh= fopen(path, "r");
	if (!fh) die_errno("open", path);
	char buf[1024];
	int got= fread(buf, 1, 1014, fh);
	if (ferror(fh)) die_errno("fread", path);
	if (fclose(fh)) die_errno("close", path);

	if (got==0) {
		printf("all tests OK.\n");
		return 0;
	} else {
		printf("test failures:\n");
		system("cat t.out");
		return 1;
	}
}
