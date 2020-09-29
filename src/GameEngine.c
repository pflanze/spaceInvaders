//GameEngine.c
/*

  The game engine keeps track of game statistics to perform movement,
  drawing, shooting.

  In general terms the statistics tracked are: enemies per row/column,
  first/last enemy per row, first enemy per column, total number of
  enemies, rows/columns with enemies alive,

*/

#include "GameEngine.h"
#include "sprites.h"
#include "Nokia5110.h"
#ifndef TEST_WITHOUT_IO
#  include "..//tm4c123gh6pm.h"
#endif
#include "random.h"
#include "utils.h"
#include "perhaps_assert.h"
#include "sounds.h"
#ifdef DEBUG
#  include <stdio.h>
#  include "pp.h"
#endif


//--------------------------General definitions---------------------------------
// (Those for Actor are grouped with the Actor implementation below)

#define REAL_MAX_EPR (MAX_ENEMY_PR-1)

//used for Convert2Distance calibration
#define Avalue 16
#define Bvalue 0

//collision offset
#define YOFFSET 6
#define XOFFSET 2

//screen specs
#define LOWERLIMIT 47
#define RIGHTLIMIT 68
#define LEFTLIMIT 0
#define TOPLIMIT 6

//ship collision line

#define SHIPCOLLISIONLINE LOWERLIMIT-PLAYERH+2

//ship midPoints
#define SHIPMIDDLE	((PLAYERW>>1)-1)

//BonusShip
#define BONUS_C_LINE TOPLIMIT+BONUSENEMYH
#define BONUSTIMING 30

//BonusShip midPoint xCoordinate
#define ENEMY_Bon_midX (BONUSENEMYW>>1)

//enemy midPoint xCoordinate
#define E_LASER_OFFX ((ENEMY30W>>1)-1)
#define E_LASER_OFFY 5


//Default values
#define	FIRST_E	0
#define	LAST_E	3


#if DRAW_ENEMIES
static
void GameEngine_enemyLaserCollisions(struct GameEngine *this);
#endif

//------------------------ActorConsts-------------------------------------------

static
const struct ActorConsts shipConsts = {
	.image[0] = playerShip0
	, .playExplosionSound = true
};

static
const struct ActorConsts shipLaserConsts = {
	.image[0] = laser0
	, .playExplosionSound = false
};

static
const struct ActorConsts enemyLaserConsts = {
	.image[0] = laser0
	, .offsetX = -5
	, .playExplosionSound = false
};

static
const struct ActorConsts bonusEnemyConsts = {
	.image[0] = smallBonusEnemy0
	, .maybeSound = &ufoLowPitch
	, .offsetX = 2
	, .offsetY = 4
	, .playExplosionSound = true
};

static
const struct ActorConsts enemy30Consts = {
	{ smallEnemy30PointA, smallEnemy30PointB }
	, .playExplosionSound = true
};
static
const struct ActorConsts enemy20Consts = {
	{ smallEnemy20PointA, smallEnemy20PointB }
	, .playExplosionSound = true
};
static
const struct ActorConsts enemy10Consts = {
	{ smallEnemy10PointA, smallEnemy10PointB }
	, .playExplosionSound = true
};

static
const struct ActorConsts smallExplosionConsts = {
	{ smallExplosion0, smallExplosion1 }
	, .maybeSound = &smallExplosion
};

static
const char *addrToActorConstsName(const struct ActorConsts *p) {
	if (! p) { return "NULL"; }
#define C(nam) if (p == &nam) return "&"#nam
	C(shipConsts);
	C(shipLaserConsts);
	C(enemyLaserConsts);
	C(bonusEnemyConsts);
	C(enemy30Consts);
	C(enemy20Consts);
	C(enemy10Consts);
	C(smallExplosionConsts);
#undef C
	return "<unknown ActorConsts>";
}

//-----------------------------Actor--------------------------------------------

#ifdef DEBUG

static
void Actor_pp(const struct Actor *this, FILE *out) {
	PP_PRINTF("(struct Actor) {");
	PP_PRINTF(" .consts = %s", addrToActorConstsName(this->consts));
	PP_PRINTF(", .origConsts = %s", addrToActorConstsName(this->origConsts));
	PP_PRINTF(", .x = %hhu", this->x);
	PP_PRINTF(", .y = %hhu", this->y);
	PP_PRINTF(", .alive = %s", bool_show(this->alive));
	PP_PRINTF(", .jk = %s", bool_show(this->jk));
	PP_PRINTF(", .frame = %i", this->frame);
	PP_PRINTF(" }");
}

static
void _Actor_pp(const void *this, FILE *out) { Actor_pp(this, out); }
    
const struct ObjectInterface Actor_ObjectInterface = {
	.pp = &_Actor_pp
};

#endif


//------------------------GameStatColumn----------------------------------------

#ifdef DEBUG

static
void GameStatColumn_pp(const struct GameStatColumn *this, FILE *out) {
	PP_PRINTF("(struct GameStatColumn) {");
	PP_PRINTF(" .fep = %hhu", this->fep);
	PP_PRINTF(", .numEnemies = %hhu", this->numEnemies);
	PP_PRINTF(" }");
}
static
void _GameStatColumn_pp(const void *this, FILE *out) { GameStatColumn_pp(this, out); }

const struct ObjectInterface GameStatColumn_ObjectInterface = {
	.pp = &_GameStatColumn_pp
};

#endif


//------------------------GameStatRow-------------------------------------------

#ifdef DEBUG

static
void GameStatRow_pp(const struct GameStatRow *this, FILE *out) {
	PP_PRINTF("(struct GameStatRow) {");
	PP_PRINTF(" .fep = %hhu", this->fep);
	PP_PRINTF(", .lep = %hhu", this->lep);
	PP_PRINTF(", .numEnemies = %hhu", this->numEnemies);
	PP_PRINTF(" }");
}
static
void _GameStatRow_pp(const void *this, FILE *out) { GameStatRow_pp(this, out); }

const struct ObjectInterface GameStatRow_ObjectInterface = {
	.pp = &_GameStatRow_pp
};

#endif



//------------------------GameEngine--------------------------------------------

// Initialize enemies
//Enemies are counted normally, top left 00
// changes: enemy[row|column].*, gameStatRow[row].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES

#define NUM_ACTORCONSTS_IN_ROW 3

static
const struct ActorConsts *actorconsts_in_row[NUM_ACTORCONSTS_IN_ROW] = {
	&enemy30Consts, &enemy20Consts, &enemy10Consts
};

EXPORTED
void GameEngine_enemyInit(struct GameEngine *this) {
	assert(ALLOC_MAXROWS <= NUM_ACTORCONSTS_IN_ROW);
	for (unsigned char row=0; row < this->maxrows; row++) {
		for (unsigned int column=0; column < MAX_ENEMY_PR; column++) {
			Actor_init(
				&this->enemy[row][column],
				// https://en.cppreference.com/w/c/language/compound_literal
				(struct Actor) {
						.consts = actorconsts_in_row[row],
						.x = 20*column,
						.y = 10 + row*10,
						.alive = true,
						.jk = false });
		}
		GameStatRow_init(
			&this->gameStatRow[row],
			(struct GameStatRow) {
					.numEnemies = MAX_ENEMY_PR,
					.fep = 0,
					.lep = 3 });
	}
}
#endif

EXPORTED
void GameEngine_shipInit(struct GameEngine *this) {
	Actor_init(
		&this->ship,
		(struct Actor) {
				.consts = &shipConsts,
				.x = 0,
				.y = 46,
				.alive = true,
				.jk = false });
}

// Function used to initialize the lasers fired by the spaceship
// changes: laser_ship[index].*
// inputs: none
// outputs: none
// assumes: na
EXPORTED
void GameEngine_shipLasersCreation(struct GameEngine *this, bool init) {
	unsigned char i;
	for (i=0; i < MAXLASERS; i++) {
		if (init || (this->laser_ship[i].alive == false)) {
			Actor_init(
				&this->laser_ship[i],
				(struct Actor) {
						.consts = &shipLaserConsts,
						.x = this->ship.x + SHIPMIDDLE,
						.y = 39,
						.alive = true,
						.jk = false });
			if (! init) {
			    break; // terminate loop when a slot is found
			}
		}
	}
}

EXPORTED
void GameEngine_shipLasers_init(struct GameEngine *this) {
	GameEngine_shipLasersCreation(this, true);
}


// Function used to initialize the lasers fired by the spaceship
// changes: laser_ship[index].*
// inputs: none
// outputs: none
// assumes: na

// XX what is the difference to GameEngine_shipLasersCreation, what is it
// for?
EXPORTED
void GameEngine_laserInit_ship2(struct GameEngine *this) {
	unsigned char i;
	unsigned int count = 0;
	
	for (i=0; i < MAXLASERS; i++) {
		if (this->laser_ship[i].alive == false) {
			unsigned char x = 0;
			switch(count){
				case 0:
					x = this->ship.x + SHIPMIDDLE;
					count++;
					break; // terminate loop when a slot is found
				case 1:
					x = this->ship.x + 2 + SHIPMIDDLE;
					count++;
					break; // terminate loop when a slot is found
				case 2:	
					x = this->ship.x + 4 + SHIPMIDDLE;
					break; // terminate loop when a slot is found
				
			}
			Actor_init(
				&this->laser_ship[i],
				(struct Actor) {
						.consts = &shipLaserConsts,
						.x = x,
						.y = 39,
						.alive = true,
						.jk = false });
		}
	}
}

// Initialize the lasers fired by the enemy ship. It selects an enemy randomly to shoot.
// changes: laser_enemy[index].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_enemyLasersCreation(struct GameEngine *this, bool init) {
	// if init == true, initializes all lasers; kinda ugly but "works for now"
    
	// Choose one of the enemies randomly:
	unsigned char randN = random_uint32(this->numLiveCols);
	// The column for the new shot:
	unsigned char columnNew	= this->liveCols[randN];
	
	if (this->gameStatColumn[columnNew].numEnemies) {
		unsigned char i;
		for (i=0; i < MAXLASERS; i++) {
			struct Actor *l= &this->laser_enemy[i];
			if (init || (l->alive == false)) {
				unsigned char row = this->gameStatColumn[columnNew].fep;
				Actor_init(
					l,
					(struct Actor) {
							.consts = &enemyLaserConsts,
							.x = this->enemy[row][columnNew].x + E_LASER_OFFX,
							.y = this->enemy[row][columnNew].y + E_LASER_OFFY,
							.alive = true,
							.jk = false });
				if (! init) {
					break; // terminate loop when a slot is found
				}
			} // otherwise laser is in use, don't issue a new one
		}
	}
}

EXPORTED
void GameEngine_enemyLasers_init(struct GameEngine *this) {
	GameEngine_enemyLasersCreation(this, true);
}

#endif

// Initialize bonus enemy
// changes: bonusEnemy.*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_BONUSENEMY	
EXPORTED
void GameEngine_bonusEnemy_init(struct GameEngine *this) {
	Actor_init(
		&this->bonusEnemy,
		(struct Actor) {
				.consts = &bonusEnemyConsts,
				.x = RIGHTLIMIT,
				.y = TOPLIMIT,
				.alive = false,
				.jk = false });
}
#endif

// Reset the values to default 
// changes: laser_enemy[i].alive, lastLine, gameStatColumn[i].(numEnemies|fep)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_defaultValues(struct GameEngine *this) {
	unsigned char i;
	//tracking defaults
	this->lastLine = this->maxrows - 1;
	//sets defaults column stats
	for (i=0; i < MAX_ENEMY_PR; i++) {
	    GameStatColumn_init(&this->gameStatColumn[i],
							(struct GameStatColumn) {
									.numEnemies = this->maxrows,
									.fep = this->lastLine });
	}
	for (i=0; i < MAX_ENEMY_PR; i++) {
		this->laser_enemy[i].alive = false;
	}

	for (i=0; i < MAXLASERS; i++) {
		this->laser_ship[i].alive = false;
	}
}
#endif

// Set values to default using "mode" switch
// changes: ship.(image|jk), functions: GameEngine_enemyShiftTrack, FirstLast, GameEngine_firstEPC
// Callers: 
// inputs: none
// outputs: none
// assumes: na
//Used by: Function name

// XX what is the difference between this and GameEngine_defaultValues
// [and GameEngine_init etc.]?

EXPORTED
void GameEngine_reset(struct GameEngine *this) {
	GameEngine_shipInit(this);
#if DRAW_ENEMIES
	GameEngine_enemyTracking_reset(this);
	GameEngine_firstEPC_reset(this);
	GameEngine_firstLast_reset(this);
#endif
}

// Update the position values of the associated objects
// inputs: INGAME
// outputs: none
// assumes: na
EXPORTED
void GameEngine_moveObjects(struct GameEngine *this) {
	GameEngine_player_move(this); // calls ADC0_in, Convert2Distance
	if (this->gStatus == INGAME) {
#if DRAW_ENEMIES
		GameEngine_enemy_move(this,
							  this->enemyTracking[0],
							  this->enemyTracking[1]);
		// ^ updates enemy coordinate
		GameEngine_laserEnemy_move(this);
#endif

#if DRAW_BONUSENEMY
		GameEngine_bonusEnemy_move(this);
#endif
		GameEngine_laserShip_move(this);
	}
}

// Get the new position value from ADC and update the spaceShip position
// changes: ship.x, 
// inputs: none
// outputs: none
// assumes: na
EXPORTED
void GameEngine_player_move(struct GameEngine *this) {
	unsigned long ADCdata;
	ADCdata = ADC0_In();
	this->ship.x = Convert2Distance(ADCdata);
}

// Update the enemy coordinates kept in enemy matrix
// changes: enemy[row][Column].[x|y]
// inputs: leftShiftColumn, lightShiftColumn
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_enemy_move(struct GameEngine *this,
						   unsigned int leftShiftColumn,
						   unsigned int lightShiftColumn) {
	unsigned char row;
	
	for (row=0; row < this->maxrows; row++) {
		// While not raching the earth
		if (this->enemy[this->lastLine][0].y < 40) {
			signed char column;
			//sets the switches to move down/left/right
			if (this->enemy[row][lightShiftColumn].x >= RIGHTLIMIT) {
				this->right = false;	//moves left
				this->down = true;
			}
			else if (this->enemy[row][leftShiftColumn].x <= LEFTLIMIT) {
				this->right = true;
				this->down = true;
			}	
			//moves left/right using the switches
			for (column=0; column < MAX_ENEMY_PR; column++) {
				if (this->right) {
					this->enemy[row][column].x += 2; // move to right
				}	
				else {
					this->enemy[row][column].x -= 2; // move to left
				}	
			}
			//moves down using the switches
			if (this->down) {
				for(column=0;column<4;column++){
					this->enemy[row][column].y += 1;
				}
				this->down = false;
			}
		}
		else {
			// Enemies have reached the earth
			this->gStatus = LOOSE;
		}
	}
}
#endif

// Update the spaceShip'2 laser coordinates
// changes: laser_ship[index].y, laser_ship[index].alive 
// inputs: none
// outputs: none
// assumes: na
EXPORTED
void GameEngine_laserShip_move(struct GameEngine *this) {
 	unsigned char i;
	for (i=0; i < MAXLASERS; i++) {
		if (this->laser_ship[i].alive) {
			this->laser_ship[i].y--;
			
			//laser out of rangef
			if (this->laser_ship[i].y <= TOPLIMIT) {
				this->laser_ship[i].alive = false;
				break;
			}
		}
	}
}

// Update the laser's friendly ship position 
// changes: laser_enemy[index].y, laser_enemy[index].alive
// inputs: none
// outputs: none
// assumes: na
//laser_enemy[i]
#if DRAW_ENEMIES
EXPORTED
void GameEngine_laserEnemy_move(struct GameEngine *this) {
 	unsigned char i;
	
	for (i=0; i < MAXLASERS; i++) {
		if (this->laser_enemy[i].alive) {
			this->laser_enemy[i].y++;
			//laser out of range
			if (this->laser_enemy[i].y > LOWERLIMIT) {//update
				this->laser_enemy[i].alive = false;
				break;
			}
		}
	}
}
#endif

#if DRAW_BONUSENEMY

EXPORTED
void GameEngine_bonusEnemy_move_reset(struct GameEngine *this) {
	this->bonusEnemy.alive = false;
}

EXPORTED
void GameEngine_bonusEnemy_move(struct GameEngine *this) {
	if (this->bonusEnemy.alive) {
		this->bonusEnemy.x--;
		if (this->bonusEnemy.x <= LEFTLIMIT) {
			this->bonusEnemy.alive = false;
		}
	}
}
#endif

// Update the object positions and send the data to the screen
// inputs: none
// outputs: none
// assumes: na
EXPORTED
void GameEngine_draw(struct GameEngine *this) {
	Nokia5110_ClearBuffer();
	
	//drawing battleship
	Actor_masterDraw(&this->ship, 0, &this->soundPlayer);

	//drawing enemies
#if DRAW_ENEMIES
	GameEngine_enemyDraw(this);
	GameEngine_laserEnemyDraw(this);
#endif
	
	//drawing laser
	GameEngine_laserShipDraw(this);

#if DRAW_BONUSENEMY		
	Actor_masterDraw(&this->bonusEnemy, 0, &this->soundPlayer);
#endif	
		
	// draw buffer
	Nokia5110_DisplayBuffer();      
}

// Send the enemy information to screen buffer
//changes: enemy[row][column].image, enemy[row][column].jk
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_enemyDraw(struct GameEngine *this) {
	unsigned int row;

	for (row=0; row < this->maxrows; row++) {
		unsigned char column;
		if (this->gStatus == INGAME) { this->frameIndex ^= 0x01; }  // 0,1,0,1,...
		for (column=0; column < 4; column++) {
			Actor_masterDraw(&this->enemy[row][column],
							 this->frameIndex,
							 &this->soundPlayer);
		}
	}
}
#endif

// Master function to draw objects
// changes: variablesChanged
// inputs: &bonusEnemy, 
// outputs: none
// assumes: na

// XX also does sound playing, detangle or rename
EXPORTED
void Actor_masterDraw(struct Actor *s,
					  unsigned int frameIndex,
					  struct SoundPlayer *soundPlayer) {
	if (s->jk) {
		switch (s->frame) {
		case 0:
		case 1:
			// switch actor to an explosion (but remember original)
			// (Need to check whether it was already done, why, due to a bug?)
			if (! s->origConsts) {
				s->origConsts = s->consts;
				s->consts = &smallExplosionConsts;
			}
			break;
		case 2:
			s->jk = false;
			s->frame = 0;
			break;
		}

		// Disable original sound if any
		{
			const struct Sound *maybeSound= s->origConsts->maybeSound;
			if (maybeSound) {
				SoundPlayer_stop_all(soundPlayer, maybeSound);
			}
		}
		// Play explosion's sound if required and available
		if (s->origConsts->playExplosionSound) {
			const struct Sound *maybeSound= s->consts->maybeSound;
			if (maybeSound) {
				SoundPlayer_play(soundPlayer, maybeSound);
			}
		}
	}
	if (s->alive) {
		const unsigned char *img = s->consts->image[frameIndex];
		if (! img) {
			img = s->consts->image[0];
		}
		Nokia5110_PrintBMP(s->x, s->y, img, 0);
	}
	else if (s->jk) {
		Nokia5110_PrintBMP(s->x + s->origConsts->offsetX,
						   s->y + s->origConsts->offsetY,
						   s->consts->image[s->frame],
						   0);
		s->frame++; // wrap-around handling is under "case 2:" above
	}
}

// Update position of alive lasers
// inputs: none
// outputs: none
// assumes: na
EXPORTED
void GameEngine_laserShipDraw(struct GameEngine *this) {
	unsigned char laserNum;
	for (laserNum=0; laserNum < MAXLASERS; laserNum++) {
		if (this->laser_ship[laserNum].alive) {
			Actor_masterDraw(&this->laser_ship[laserNum],
							 0,
							 &this->soundPlayer);
		}
	}
}

// Send current position to buffer
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_laserEnemyDraw(struct GameEngine *this) {
	unsigned char laserNum;
	for (laserNum=0; laserNum < MAXLASERS; laserNum++) {
		Actor_masterDraw(&this->laser_enemy[laserNum],
						 0,
						 &this->soundPlayer);
	}
}
#endif

#if DRAW_ENEMIES
EXPORTED
void GameEngine_enemyTracking_reset(struct GameEngine *this) {
	this->enemyTracking[0] = FIRST_E;		
	this->enemyTracking[1] = LAST_E;
	this->lowest = FIRST_E;	
	this->highest = LAST_E;
}

// Keep track of the leftmost and rightmost Enemies.
EXPORTED
void GameEngine_enemyShiftTrack(struct GameEngine *this,
								unsigned int localAliveRows) {
	switch(localAliveRows){
		case 1:
			if (this->gameStatRow[this->lastLine].numEnemies == 1) {
				this->enemyTracking[0] = this->gameStatRow[this->lastLine].fep;
				this->enemyTracking[1] = this->gameStatRow[this->lastLine].fep;
			}
			else {
				this->enemyTracking[1] = this->gameStatRow[this->lastLine].lep;
				this->enemyTracking[0] = this->gameStatRow[this->lastLine].fep;
			}
			break;
		default:{
			unsigned int row = 0;
			this->lowest 	= this->gameStatRow[row].fep;
			this->highest	= this->gameStatRow[row].lep;
			row = 1;
			while (row <= this->maxrows - 1) {//change
				if (this->gameStatRow[row].fep < this->lowest) {
					this->lowest = this->gameStatRow[row].fep;
				}
				if (this->gameStatRow[row].lep > this->highest) {
					this->highest = this->gameStatRow[row].lep;
				}
				row++;
			}
			this->enemyTracking[1] = this->highest;
			this->enemyTracking[0] = this->lowest;
		}	
	}
}
#endif

//-----------------------------------------------------ADC----------------------

// Convert the sample value to an equivalent distance
// inputs: sample
// outputs: none
// assumes: na
EXPORTED
unsigned long Convert2Distance(unsigned long sample){
	return (Avalue*(sample) >> 10)+Bvalue; // Needs recalibration
}


// ADC converter
// inputs: none
// outputs: result (of the conversion)
// assumes: na

#ifdef TEST_WITHOUT_IO
unsigned long ADC0_SSFIFO3_R=0;
#endif

EXPORTED
unsigned long ADC0_In(void) {
#ifndef TEST_WITHOUT_IO
	unsigned long result;
	ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
	while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
	result = ADC0_SSFIFO3_R & 0xFFF; // 3) read result
	ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
	return result;
#else
	return ADC0_SSFIFO3_R;
#endif
}

//----------------------------------------------COLLISIONS----------------------

// Check collision detection: enemy lasers, ship lasers, laser on laser
// inputs: none
// outputs: none
// assumes: na
// Note: the result pointer must be array to make the collision result homogeneous
EXPORTED
void GameEngine_collisions(struct GameEngine *this) {
#if DRAW_BONUSENEMY
	GameEngine_bonusLaserCollision(this);
#endif
	
#if DRAW_ENEMIES
#if	GODMODE
	GameEngine_enemyLaserCollisions(this);
#endif	
	GameEngine_playerLaserCollisions(this);
	GameEngine_laserCollision(this);
#endif
}

// Check if the laser have a hit to an enemy (for every laser)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_playerLaserCollisions(struct GameEngine *this) {
	unsigned char laserNum = 0; 
	//each laser is checked for a collition
	for (laserNum=0; laserNum < MAXLASERS; laserNum++) {
		if ((this->laser_ship[laserNum].alive) &&
		    (this->enemy[this->lastLine][0].y + YOFFSET >=
		     this->laser_ship[laserNum].y)) {
			// found a line with enemies>>start calculating
			// calculate enemy zone(grouping)
			GameEngine_enemyscanY(this, laserNum);
		}	
	}
}
#endif

// Scans for an enemy line (y-axis), then send a request to scan for
// an enemy on the x-axis. Update the enemy line values
// inputs: lastLine, laserNum
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_enemyscanY(struct GameEngine *this,
						   unsigned int laserNum) {
	signed char row = this->lastLine;
	bool found = false;
	bool exit = false;
	
	unsigned char ELL[ALLOC_MAXROWS];		//enemy line low
	unsigned char ELH[ALLOC_MAXROWS];		//enemy line high
	
	//it creates an array with thresholds
	for (row= this->lastLine; row >= 0; row--) {
		ELL[row] = this->enemy[row][0].y + YOFFSET;
		ELH[row] = this->enemy[row][0].y;
	}
	
	row = this->lastLine;//temporal variable reset
		
	while((! found) && (! exit)) {
		assert(row >= 0);
		if (this->laser_ship[laserNum].y > ELL[row]) {
			exit = true;
		}
		else if (this->laser_ship[laserNum].y < ELH[row]) {
			row--;
		}
		else {
			GameEngine_enemyscanX(this, row, laserNum);
			found = true;
		}
		exit = true;
	}
}	
#endif

// Keep track of the last enemy line
// changes: lastLine
// inputs: none
// outputs: lastLine
// assumes: na
#if DRAW_ENEMIES
static
void GameEngine_update_lastLine(struct GameEngine *this) {
	while (this->gameStatRow[this->lastLine].numEnemies == 0) {
		if (this->lastLine == 0)
			break; // correct?
		this->lastLine--;
	}
}
#endif

// Scan for a enemy collition on a single row (x axis)
// changes: laser_ship[index].alive, enemy[row][column].alive, enemy[row][column].jk
// inputs: row, laserNum
// outputs: enemyDestroyed(0:1)
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_enemyscanX(struct GameEngine *this,
						   unsigned int row,
						   unsigned int laserNum) {
	unsigned char column; 
	
	for (column=0; column <= this->gameStatRow[row].lep; column++) {
		if (this->enemy[row][column].alive) {
			//checking x coordinate of each active laser against each enemy
			signed char enemyInRange =
				(this->enemy[row][column].x
				 + E_LASER_OFFX
				 - this->laser_ship[laserNum].x);
			enemyInRange = absValue(enemyInRange);
			if (enemyInRange <= E_LASER_OFFX) {	
				unsigned int alive_rows;
				this->laser_ship[laserNum].alive = false;
				this->enemy[row][column].alive = false;
				this->enemy[row][column].jk = true;
				alive_rows = GameEngine_firstLast(this, row, column);
				GameEngine_update_lastLine(this);
				//updates 
				GameEngine_enemyShiftTrack(this, alive_rows);
				GameEngine_firstEPC(this); //update point
				break; //return????
			}
		}
	}	
}
#endif

// Detect the collision between laserEnemy and our ship
// changes: ship.alive, ship.jk
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
static
void GameEngine_enemyLaserCollisions(struct GameEngine *this) {
	if (this->ship.alive) {
		unsigned char i = 0;
		for (i=0; i < MAXLASERS; i++) {
			//check agains the ship
			if (this->laser_enemy[i].alive &&
			    (this->laser_enemy[i].y > SHIPCOLLISIONLINE)){
				signed char collision = this->ship.x
					+ SHIPMIDDLE
					- this->laser_enemy[i].x;
				collision = absValue(collision);
				if (collision <= SHIPMIDDLE) {
					this->laser_enemy[i].alive = false;
					this->laser_enemy[i].jk = false;
					this->ship.alive = false;
					this->ship.jk = true;
				}
			}
		}
	}
}
#endif

// Detect collisions between lasers
// changes: variablesChanged
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
EXPORTED
void GameEngine_laserCollision(struct GameEngine *this) {
	unsigned char lasernumEnemy;
	//checks collision course
	for (lasernumEnemy=0; lasernumEnemy < MAXLASERS; lasernumEnemy++) {
		if (this->laser_enemy[lasernumEnemy].alive) {
			unsigned char lasernumShip = 0; //avoids unnecessary comparison
			for (lasernumShip=0; lasernumShip < MAXLASERS; lasernumShip++) {
				// avoid unnecessary comparison
				if (this->laser_ship[lasernumShip].alive) {
					signed char xDistance =
						this->laser_enemy[lasernumEnemy].x
						- this->laser_ship[lasernumShip].x;
					if ((absValue(xDistance)<2) &&
					    (this->laser_ship[lasernumShip].alive)) {
						signed char yDistance =
							(this->laser_enemy[lasernumEnemy].y
							 + LASERH)
							> this->laser_ship[lasernumShip].y;
						// ^ crossOver each other
						if (yDistance) {
							this->laser_enemy[lasernumEnemy].alive =
								false;
							this->laser_enemy[lasernumEnemy].jk =
								true;
							this->laser_ship[lasernumShip].alive =
								false;
						}
					}
				}
			}
		}
	}
}
#endif

// Detect collision between shipLaser and bonusEnemy
// changes: bonusEnemy (jk, alive), laser_ship (jk, alive)
// inputs: none
// outputs: none
// assumes: na
//notes: BonusLaserCollision does not need return (game does not terminate)
#if DRAW_BONUSENEMY
EXPORTED
void GameEngine_bonusLaserCollision(struct GameEngine *this) {
	if (this->bonusEnemy.alive) {
		unsigned char laserNumber = 0;
		for (laserNumber=0; laserNumber < MAXLASERS; laserNumber++) {
			if (this->laser_ship[laserNumber].alive) {
				if (this->laser_ship[laserNumber].y <= BONUS_C_LINE) {
					// any of the lasers passes certain threshold
					signed char xDistance =
						this->bonusEnemy.x
						+ ENEMY_Bon_midX
						- this->laser_ship[laserNumber].x;
					xDistance = absValue(xDistance);
					if (xDistance < ENEMY_Bon_midX) {
						this->bonusEnemy.alive = false;
						this->bonusEnemy.jk = true;
						this->laser_ship[laserNumber].alive = false;
						this->laser_ship[laserNumber].jk = true;
					}
				}
			}
		}
	}
}
#endif

//-----------------------------------------GAME STATS---------------------------

#if DRAW_ENEMIES

EXPORTED
void GameEngine_firstLast_reset(struct GameEngine *this) {
	this->enemyCount = this->maxrows * MAX_ENEMY_PR; // see COPYPASTE
	{	//liverows[] defaults
		unsigned char i;
		for (i=0; i < this->maxrows; i++) {
			this->rowAlive[i] = true;
		}
	}
}

//this function should keep track of:
//					the number of enemies on each row
//					It is called by GameEngine_enemyscanX to update gameStatRow
//					it is also used to update general game stats
// changes: gameStatRow[row].*, gameStatColumn[column].numEnemies, enemyCount, rowAlive[row]
// inputs: row, column, mode
// outputs: none
// assumes: na
EXPORTED
unsigned int GameEngine_firstLast(struct GameEngine *this,
								  unsigned int row,
								  unsigned int column) {
	bool lastCheck = false;
	
	this->gameStatRow[row].numEnemies--;
	this->gameStatColumn[column].numEnemies--;
	this->enemyCount--;
	
	if (this->enemyCount == 0) {
		this->rowAlive[row] = false;
		// ^ needed only to update stats before quitting, good for debugging
		GameEngine_setStatus(this, WIN);
	}
	else {
		if (this->gameStatRow[row].numEnemies == 1) {
			lastCheck = true;			//Does forward checking only
		}
		else if (this->gameStatRow[row].numEnemies == 0) {
			this->rowAlive[row] = false;
		}

		if (this->gameStatRow[row].numEnemies) {
			unsigned char column=0;
			bool firstCheck = false;
			for (column=0; column < MAX_ENEMY_PR; column++) {
				if ((! firstCheck) &&
				    (this->enemy[row][column].alive)) {
					// ^ counts forward, check = 0 >> keeps checking
					this->gameStatRow[row].fep = column;
					firstCheck = true;
					if (this->gameStatRow[row].numEnemies == 1) {
						this->gameStatRow[row].lep =
							this->gameStatRow[row].fep;
					}
				}
				
				if ((! lastCheck) &&
				    (this->enemy[row][REAL_MAX_EPR-column].alive)) {
					// ^ counts backwards
					this->gameStatRow[row].lep = REAL_MAX_EPR-column;
					lastCheck = true;
				}
				
				if (firstCheck && lastCheck) {
					break;
				}
			}
		}
	}

	{
		unsigned int alr_counter=0;
		{
			unsigned char i;
			for (i=0; i <= this->maxrows - 1; i++) {
				if (this->rowAlive[i]) {
					alr_counter++;
				}
			}
		}
		return alr_counter;
	}
}
#endif

// Keep track of the first enemy per column,
//    Used for: - knowing how far enemies should move (before switching
//                direction)
// changes: gameStatColumn[column].(fep|numEnemies),liveCols[numLiveCols], numLiveCols
// Callers: EnemyLaserInit, GameEngine_enemyscanX
// outputs: numLiveCols
// assumes: na
#if DRAW_ENEMIES

EXPORTED
void GameEngine_firstEPC_reset(struct GameEngine *this) {
	//sets defaults
	unsigned char i;
	this->numLiveCols = MAX_ENEMY_PR;
	for (i=0; i < MAX_ENEMY_PR; i++) {
		this->liveCols[i] = i;
	}
}

EXPORTED
void GameEngine_firstEPC(struct GameEngine *this) {
	unsigned char numLiveCols = 0;

	// read left -> right, down -> up
	for (unsigned char column=0; column < MAX_ENEMY_PR; column++) {
		if (this->gameStatColumn[column].numEnemies == 0) {
			continue;
		}

		// start from last known position
		unsigned int row = this->gameStatColumn[column].fep;
		assert(row < this->maxrows);
	
		this->liveCols[numLiveCols] = column;
		numLiveCols++;
		
		// find the first (closest to player) live enemy on a column and update
		// the column's gameStatColumn to refer to it
		while (1) {
			if (this->enemy[row][column].alive) {
				this->gameStatColumn[column].fep = row;
				break;
			}
			if (row > 0)
				row--;
			else
				break;
		}
	}
	this->numLiveCols = numLiveCols;
}
#endif

//----------------------------------Miscellaneus--------------------------------

// Create the enemy bonus ship, and time the reappearence
// changes: na
// inputs: none
// outputs: none
// assumes: na
#if DRAW_BONUSENEMY
EXPORTED
void GameEngine_bonusEnemyCreate(struct GameEngine *this) {
	
	if ((this->bonusEnemy.alive == false) &&
		(this->localCounter >= BONUSTIMING)){
		GameEngine_bonusEnemy_init(this);
		this->bonusEnemy.alive = true;
		SoundPlayer_play(&this->soundPlayer, &ufoLowPitch);
		this->localCounter = 0;
	}
		
	if (this->bonusEnemy.alive == false) {
		this->localCounter++;
	}
}
#endif


EXPORTED
signed int absValue(signed int value){
	return ((value<0) ? -value:value);
}

// Systick sets the game status on the Engine
// Callers: systick, FirstLast, EnemyLaserCollisions
EXPORTED
void GameEngine_setStatus(struct GameEngine *this, const unsigned int v) {
	this->gStatus = v;
}
EXPORTED
unsigned int GameEngine_getStatus(struct GameEngine *this) {
	return this->gStatus;
}


#ifdef DEBUG

static
void GameEngine_pp(const struct GameEngine *this, FILE *out) {
	int maxrows= this->maxrows; // XX or ALLOC_MAXROWS ?

	PP_PRINTF("(struct GameEngine) {");
	PP_PRINTF(" .soundPlayer = ");
	V(pp, &this->soundPlayer, out);
	PP_PRINTF(", .gStatus = %u", this->gStatus);
	PP_PRINTF(", .maxrows = %u", this->maxrows);
	PP_PRINTF(", .lastLine = %u", this->lastLine);
	PP_PRINTF(", .enemyCount = %hhu", this->enemyCount);

	PP_PRINTF(", .gameStatColumn = { ");
	{
		bool first= true;
		for (int i=0; i< MAX_ENEMY_PR; i++) {
			if (! first) { PP_PRINTF(", "); }
			first = false;
			V(pp, &this->gameStatColumn[i], out);
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .gameStatRow = { ");
	{
		bool first= true;
		for (int i=0; i < maxrows; i++) {
			if (! first) { PP_PRINTF(", "); }
			first = false;
			V(pp, &this->gameStatRow[i], out);
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .enemy = { ");
	{
		bool first= true;
		for (int i=0; i < maxrows; i++) {
			for (int j=0; j < MAX_ENEMY_PR; j++) {
				if (! first) { PP_PRINTF(", ");}
				first = false;
				V(pp, &this->enemy[i][j], out);
			}
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .laser_enemy = { ");
	{
		bool first= true;
		for (int i=0; i < MAXLASERS; i++) {
			if (! first) { PP_PRINTF(", ");}
			first = false;
			V(pp, &this->laser_enemy[i], out);
		}
	}
	PP_PRINTF(" }");
	
	PP_PRINTF(", .ship = ");
	V(pp, &this->ship, out);

	PP_PRINTF(", .laser_ship = { ");
	{
		bool first= true;
		for (int i=0; i< MAXLASERS; i++) {
			if (! first) {PP_PRINTF(", ");}
			first = false;
			V(pp, &this->laser_ship[i], out);
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .bonusEnemy = ");
	V(pp, &this->bonusEnemy, out);

	PP_PRINTF(", .right = %s", bool_show(this->right));
	PP_PRINTF(", .down = %s", bool_show(this->down));
	PP_PRINTF(", .frameIndex = %hhu", this->frameIndex);

	PP_PRINTF(", .enemyTracking = { ");
	{
		bool first= true;
		for (int i=0; i< 2; i++) {
			if (! first) {PP_PRINTF(", ");}
			first = false;
			PP_PRINTF("%u", this->enemyTracking[i]);
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .lowest = %hhu", this->lowest);
	PP_PRINTF(", .highest = %hhu", this->highest);

	PP_PRINTF(", .rowAlive = { ");
	{
		bool first= true;
		for (int i=0; i < maxrows; i++) {
			if (! first) {PP_PRINTF(", ");}
			first = false;
			PP_PRINTF("%s", bool_show(this->rowAlive[i]));
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .numLiveCols = %u", this->numLiveCols);

	PP_PRINTF(", .liveCols = { ");
	{
		bool first= true;
		for (int i=0; i< MAX_ENEMY_PR; i++) {
			if (! first) {PP_PRINTF(", ");}
			first = false;
			PP_PRINTF("%u", this->liveCols[i]);
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .localCounter = %hhu", this->localCounter);

	PP_PRINTF(" }");
}
static void _GameEngine_pp(const void *this, FILE *out) { GameEngine_pp(this, out); }

const struct ObjectInterface GameEngine_ObjectInterface = {
	.pp = &_GameEngine_pp
};

#endif


EXPORTED
void GameEngine_init(struct GameEngine *this,
					 unsigned int max_number_of_enemy_rows) {
#ifdef DEBUG
	this->vtable = &GameEngine_ObjectInterface;
#endif

	SoundPlayer_init(&this->soundPlayer);

	this->gStatus = STANDBY;
	assert(max_number_of_enemy_rows <= ALLOC_MAXROWS);
	this->maxrows= max_number_of_enemy_rows;
#if DRAW_ENEMIES
	this->lastLine= this->maxrows - 1;
	this->enemyCount= this->maxrows * MAX_ENEMY_PR; // COPYPASTE
#endif

	GameEngine_reset(this);
#if DRAW_ENEMIES
	GameEngine_enemyInit(this);
	// must call defaultValues and firstEPC *before* enemyLaserInit
	GameEngine_defaultValues(this);
	// GameEngine_firstEPC_reset(this); included in GameEngine_reset
	GameEngine_enemyLasers_init(this);
#endif
#if DRAW_BONUSENEMY
	GameEngine_bonusEnemy_init(this);
#endif

	// must call GameEngine_shipInit before
	// GameEngine_shipLasersCreation and GameEngine_laserInit_ship2
	GameEngine_shipInit(this);
	GameEngine_shipLasers_init(this);
	GameEngine_laserInit_ship2(this); // has no effect, not sure why. everything
									  // *is* alive at first.

	this->right = true;
	this->down = false;
	this->frameIndex = 0;
	GameEngine_enemyTracking_reset(this);

#if DRAW_BONUSENEMY
	this->localCounter = 0;
#endif
}


//-----------------------------------------------TODOS--------------------------
/*
changes:
	*add sound:
		-lab13
Improve firing: adding firing secuences

-------------------------------------------Var Map Globals----------------------
I created this section to identify the needs to re-scoping.
>: Static value modified to default(Special care or consideration)

Reason to remain: There are several function that need this variable
(a entire branch Collision), then it updates at the end of the branch
(Verify_lastLine), and it is also used on a separate branch that needs
the value already updated.

Var: lastLine
updated@:Verify_lastLine
Functions:
		Verify_lastLine
		GameEngine_enemyscanX
		GameEngine_enemyscanY
		defaultValues
		GameEngine_laserEnemy_move
		GameEngine_enemyShiftTrack
		PlayerLaserCollisions
		
		notes:
		End game@: FirstLast, EnemyLaserCollisions@GameEngine_masterDraw
		
		compiler:
		http://www.keil.com/support/man/docs/armcc/armcc_chr1359124211145.htm
		
		changes?
		1.- not bit padding, it is not worth it for now...unless perhaps tested with the optimization. In any case leave it for the end.
		2.- I was not able to determine what frunction definitions can be elinated
		3.- NO inline functions"_inline"
		
		Changes!!!!
		missing some passing by reference
		
		find sounds@
		C:\WinSSDtemp\Home\desktop\KeepUpdating\Labware\Lab13_DAC
		C:\WinSSDtemp\Home\desktop\KeepUpdating\Labware\Lab15_SpaceInvaders\Lab15Files\Sounds
		
*/
