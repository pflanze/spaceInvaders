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
void game_step(struct Game *game,
			   FILE *step_dump_fh, FILE *sound_dump_fh, FILE *sound_raw_fh) {
	// one step in the game, except for the audio sample handler:
	PP_TO(&game->spaceInvaders, step_dump_fh);
	SpaceInvaders_step(&game->spaceInvaders);
	SpaceInvaders_main_update_LCD(&game->spaceInvaders);
	GPIO_PORTE_DATA_R=0; // revert the push button to off
	game->frame_number++;

	// audio sample handler:
	PP_TO(&game->spaceInvaders, sound_dump_fh);
	struct SoundPlayer *soundPlayer = &game->spaceInvaders.gameEngine.soundPlayer;
	// 30 Hz * 367 = 11010 Hz sampling rate
	REPEAT(367) {
		if (soundPlayer->timerRunning) {
			PP_TO(soundPlayer, sound_dump_fh);
			SoundPlayer_step(soundPlayer);
		} else {
			fprintf(sound_dump_fh, ".timerRunning = false\n");
		}
		fputc((256 / (MAX_SAMPLE + 1)) * soundPlayer->currentSample,
			  sound_raw_fh);
	}
}


#define LET_XMALLOC(var, type) type *var = xmalloc(sizeof(*var))

#define OUTFILE_PATHSIZ 100
struct OutFile {
	char path[OUTFILE_PATHSIZ];
	FILE *out;
};

static
struct OutFile *OutFile_xopen(unsigned int i, const char *name) {
	LET_XMALLOC(this, struct OutFile);
	snprintf(this->path, OUTFILE_PATHSIZ, "%i-%s", i, name);
	this->out = fopen(this->path, "w");
	if (! this->out) {
		die_errno("open", this->path);
	}
	return this;
}

static
void OutFile_xclose_and_free(struct OutFile *this) {
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

	struct OutFile *stepDumpFile = OutFile_xopen(max_number_of_enemy_rows, "step.dump");
	struct OutFile *soundDumpFile = OutFile_xopen(max_number_of_enemy_rows, "sound.dump");
	struct OutFile *soundRawFile = OutFile_xopen(max_number_of_enemy_rows, "sound.raw");

	game.max_number_of_enemy_rows= max_number_of_enemy_rows;
	game.frame_number= -1;

	random_init(223412);

	SpaceInvaders_init(&game.spaceInvaders,
					   max_number_of_enemy_rows);
	ADC0_SSFIFO3_R= 0;

#define GAME_STEP game_step(&game, stepDumpFile->out, soundDumpFile->out, \
							soundRawFile->out)
	GAME_STEP;
	game_screen_write(&game);

	REPEAT(8) {
		GPIO_PORTE_DATA_R=1;
		REPEAT(10) {
			GAME_STEP;
			game_screen_write(&game);
		}
	}

	ADC0_SSFIFO3_R= 2000;

	REPEAT(8) {
		GPIO_PORTE_DATA_R=1;
		REPEAT(10) {
			GAME_STEP;
			game_screen_write(&game);
		}
	}
#undef GAME_STEP

	OutFile_xclose_and_free(soundRawFile);
	OutFile_xclose_and_free(soundDumpFile);
	OutFile_xclose_and_free(stepDumpFile);
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
