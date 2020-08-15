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
#include "assert.h"
#include "Sound.h"
#ifdef DEBUG
#include <stdio.h>
#include "pp.h"
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
#define OFFSETEXPLOSIONX 2
#define OFFSETEXPLOSIONY 4

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

//-----------------------------Actor--------------------------------------------

// Actor IDs
#define ID_SHIP        0
#define ID_ENEMY       1
#define ID_BONUS       2
#define ID_E_LASER     3
#define ID_S_LASER     4

#ifdef DEBUG

const char* Actor_id_string(struct Actor *this) {
	switch (this->id) {
	case ID_SHIP: return "ID_SHIP";
	case ID_ENEMY: return "ID_ENEMY";
	case ID_BONUS: return "ID_BONUS";
	case ID_E_LASER: return "ID_E_LASER";
	case ID_S_LASER: return "ID_S_LASER";
	}
	return "<invalid id>";
}

static
void Actor_pp(struct Actor* this, FILE* out) {
	PP_PRINTF("(struct Actor) {");
	PP_PRINTF(" .x = %hhu", this->x);
	PP_PRINTF(", .y = %hhu", this->y);
	PP_PRINTF(", .image = {");
	{
		bool first= true;
		for (int i=0; i<2; i++) {
			if (! first) { PP_PRINTF(","); }
			first = false;
			{
				const unsigned char* p= this->image[i];
				const char* nam= addrToSpriteName(p);
				if (nam) {
					PP_PRINTF(" %s", nam);
				} else {
					PP_PRINTF(" %p", p);
				}
			}
		}
	}
	PP_PRINTF(" }");
	PP_PRINTF(", .alive = %s", bool_show(this->alive));
	PP_PRINTF(", .jk = %s", bool_show(this->jk));
	PP_PRINTF(", .id = %s", Actor_id_string(this));
	PP_PRINTF(" }");
}

static
void _Actor_pp(void* this, FILE* out) { Actor_pp(this, out); }
    
const struct ObjectInterface Actor_ObjectInterface = {
	.pp = &_Actor_pp
};

#endif


//------------------------GameStatColumn----------------------------------------

#ifdef DEBUG

static
void GameStatColumn_pp(struct GameStatColumn* this, FILE* out) {
	PP_PRINTF("(struct GameStatColumn) {");
	PP_PRINTF(" .fep = %hhu", this->fep);
	PP_PRINTF(", .epc = %hhu", this->epc);
	PP_PRINTF(" }");
}
static
void _GameStatColumn_pp(void* this, FILE* out) { GameStatColumn_pp(this, out); }

const struct ObjectInterface GameStatColumn_ObjectInterface = {
	.pp = &_GameStatColumn_pp
};

#endif


//------------------------GameStatRow-------------------------------------------

#ifdef DEBUG

static
void GameStatRow_pp(struct GameStatRow* this, FILE* out) {
	PP_PRINTF("(struct GameStatRow) {");
	PP_PRINTF(" .fep = %hhu", this->fep);
	PP_PRINTF(", .lep = %hhu", this->lep);
	PP_PRINTF(", .epr = %hhu", this->epr);
	PP_PRINTF(" }");
}
static
void _GameStatRow_pp(void* this, FILE* out) { GameStatRow_pp(this, out); }

const struct ObjectInterface GameStatRow_ObjectInterface = {
	.pp = &_GameStatRow_pp
};

#endif



//-----------------------------------------------------------INIT---------------

// Initialize enemies
//Enemies are counted normally, top left 00
// changes: enemy[row|column].*, gameStatRow[row].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyInit(struct GameEngine *this) {
	unsigned char row;
	for (row=0; row < this->maxrows; row++) {
		unsigned int column;
		for (column=0; column < MAX_ENEMY_PR; column++) {
			const unsigned char* image0;
			const unsigned char* image1;
			switch(row) {
				case 0:
					image0 = smallEnemy30PointA;
					image1 = smallEnemy30PointB;
					break;
				case 1:
					image0 = smallEnemy20PointA;
					image1 = smallEnemy20PointB;
					break;
				case 2:
					image0 = smallEnemy10PointA;
					image1 = smallEnemy10PointB;
					break;
			}
			Actor_init(
				&this->enemy[row][column],
				// https://en.cppreference.com/w/c/language/compound_literal
				(struct Actor) {
						.x = 20*column,
						.y = 10 + row*10,
						.image[0] = image0,
						.image[1] = image1,
						.alive = true,
						.jk = false,
						.id = ID_ENEMY});
		}
		GameStatRow_init(
			&this->gameStatRow[row],
			(struct GameStatRow) {
					.epr = MAX_ENEMY_PR,
					.fep = 0,
					.lep = 3});
	}
}
#endif

void GameEngine_shipInit(struct GameEngine *this) {
	Actor_init(
		&this->ship,
		(struct Actor) {
				.x = 0,
				.y = 46,
				.image[0] = playerShip0,
				.alive = true,
				.jk = false,
				.id = ID_SHIP});
}

// Function used to initialize the lasers fired by the spaceship
// changes: laser_ship[index].*
// inputs: none
// outputs: none
// assumes: na
void GameEngine_shipLasersCreation(struct GameEngine *this, bool init) {
	unsigned char i;
	for (i=0; i < MAXLASERS; i++) {
		if (init || (this->laser_ship[i].alive == false)) {
			Actor_init(
				&this->laser_ship[i],
				(struct Actor){
						.x = this->ship.x + SHIPMIDDLE,
						.y = 39,
						.image[0] = laser0,
						.alive = true,
						.id = ID_S_LASER,
						.jk = false});
			if (! init) {
			    break; // terminate loop when a slot is found
			}
		}
	}
}

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
void GameEngine_laserInit_ship2(struct GameEngine *this) {
	unsigned char i;
	unsigned int count = 0;
	
	for (i=0; i < MAXLASERS; i++) {
		if (this->laser_ship[i].alive == false) {
			switch(count){
				case 0:
					this->laser_ship[i].x = this->ship.x + SHIPMIDDLE;
					count++;
					break; // terminate loop when a slot is found
				case 1:
					this->laser_ship[i].x = this->ship.x + 2 + SHIPMIDDLE;
					count++;
					break; // terminate loop when a slot is found
				case 2:	
					this->laser_ship[i].x = this->ship.x + 4 + SHIPMIDDLE;
					break; // terminate loop when a slot is found
				
			}
			this->laser_ship[i].y = 39;
			this->laser_ship[i].image[0] = laser0;
			this->laser_ship[i].alive = true;
			this->laser_ship[i].id = ID_S_LASER;
			this->laser_ship[i].jk = false;
		}
	}
}

// Initialize the lasers fired by the enemy ship. It selects an enemy randomly to shoot.
// changes: laser_enemy[index].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyLasersCreation(struct GameEngine *this, bool init) {
	// if init == true, initializes all lasers; kinda ugly but "works for now"
    
	// Choose one of the enemies randomly:
	unsigned char randN = (Random32()>>24) % this->liveCols;
	// ^ generates number [0-aliveCols]
	unsigned char columnNew	= this->alColsMat[randN];
	
	if (this->gameStatColumn[columnNew].epc) {
		unsigned char i;
		for (i=0; i < MAXLASERS; i++) {
			struct Actor *l= &this->laser_enemy[i];
			if (init || (l->alive == false)) {
				unsigned char row = this->gameStatColumn[columnNew].fep;
				Actor_init(
					l,
					(struct Actor){
							.x = this->enemy[row][columnNew].x + E_LASER_OFFX,
							.y = this->enemy[row][columnNew].y + E_LASER_OFFY,
							.image[0] = laser0,
							.alive = true,
							.jk = false,
							.id = ID_E_LASER});
				if (! init) {
					break; // terminate loop when a slot is found
				}
			} // otherwise laser is in use, don't issue a new one
		}
	}
}

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
void GameEngine_bonusEnemy_init(struct GameEngine *this) {
	Actor_init(
		&this->bonusEnemy,
		(struct Actor){
				.x = RIGHTLIMIT,
				.y = TOPLIMIT,
				.image[0] = smallBonusEnemy0,
				.alive = false,
				.jk = false,
				.id = ID_BONUS});
}
#endif

//-----------------------------------------------------------DEFAULT VALUES-----

// Reset the values to default 
// changes: laser_enemy[i].alive, lastLine, gameStatColumn[i].(epc|fep)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_defaultValues(struct GameEngine *this) {
	unsigned char i;
	//tracking defaults
	this->lastLine = this->maxrows - 1;
	//sets defaults column stats
	for (i=0; i < MAX_ENEMY_PR; i++) {
	    GameStatColumn_init(&this->gameStatColumn[i],
							(struct GameStatColumn){
									.epc = this->maxrows,
									.fep = this->lastLine});
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

void GameEngine_reset(struct GameEngine *this) {
	GameEngine_shipInit(this);
#if DRAW_ENEMIES
	GameEngine_enemyTracking_reset(this);
	GameEngine_firstEPC_reset(this);
	GameEngine_firstLast_reset(this);
#endif
}

//--------------------------------------------------------------MOVE OBJECTS----

// Update the position values of the associated objects
// inputs: INGAME
// outputs: none
// assumes: na
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
		GameEngine_bonusEnemy_Move(this, INGAME);
#endif
		GameEngine_laserShip_move(this);
	}
}

// Get the new position value from ADC and update the spaceShip position
// changes: ship.x, 
// inputs: none
// outputs: none
// assumes: na
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
void GameEngine_laserEnemy_move(struct GameEngine *this) {
 	unsigned char i;
	
	for (i=0; i < MAXLASERS; i++) { // XXX use runtime value not MAXLASERS, RIGHT?
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
void GameEngine_bonusEnemy_Move(struct GameEngine *this, unsigned int mode) {
	if (mode == RESET) {
		this->bonusEnemy.alive = false;
		return;
	}
	if (this->bonusEnemy.alive) {
		this->bonusEnemy.x--;
		if (this->bonusEnemy.x <= LEFTLIMIT) {
			this->bonusEnemy.alive = false;
		}
	}
}
#endif

//-------------------------------------------DRAWING----------------------------

// Update the object positions and send the data to the screen
// inputs: none
// outputs: none
// assumes: na
void GameEngine_draw(struct GameEngine *this) {
	Nokia5110_ClearBuffer();
	
	//drawing battleship
	GameEngine_masterDraw(this, &this->ship, 0);

	//drawing enemies
#if DRAW_ENEMIES
	GameEngine_enemyDraw(this);			//Uses GameEngine_masterDraw
	GameEngine_laserEnemyDraw(this);		//Uses GameEngine_masterDraw
#endif
	
	//drawing laser
	GameEngine_laserShipDraw(this);		//uses GameEngine_masterDraw

#if DRAW_BONUSENEMY		
	GameEngine_masterDraw(this, &this->bonusEnemy, 0);
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
void GameEngine_enemyDraw(struct GameEngine *this) {
	signed char row;

	for (row=0; row < this->maxrows; row++) {
		unsigned char column;
		if (this->gStatus == INGAME) { this->frameCount ^= 0x01; }  // 0,1,0,1,...
		for (column=0; column < 4; column++) {
			GameEngine_masterDraw(this,
								  &this->enemy[row][column],
								  this->frameCount);
		}
	}
}
#endif

// Master function to draw objects
// changes: variablesChanged
// inputs: &bonusEnemy, 
// outputs: none
// assumes: na

// XX should probably change this into a Actor_draw procedure and move
// the GameEngine parts out.
void GameEngine_masterDraw(struct GameEngine *this,
						   struct Actor *s,
						   unsigned int frameCount) {
	signed char offsetX = 0;
	signed char offsetY = 0;
	
	if (s->jk) {
		//used to change explosions offset values
		if (s->id == ID_BONUS) {	//BONUS
			Sound_stop_all(&ufoLowPitch);
			Sound_Play(&smallExplosion);
			offsetX = OFFSETEXPLOSIONX;
			offsetY = OFFSETEXPLOSIONY;
		}
		else if (s->id == ID_SHIP) {
			Sound_Play(&smallExplosion);
		}
		else if (s->id == ID_E_LASER) {
			offsetX = -5;
		}
		else if (s->id == ID_ENEMY) {
			Sound_Play(&smallExplosion);
		}
		
		switch (this->frame) {
			case 0:
				s->image[0] = smallExplosion0;
				s->image[1] = smallExplosion1;
				break;
			case 1:
				s->image[0] = smallExplosion0;
				s->image[1] = smallExplosion1;
				break;
			case 2:
				s->jk = false;
				this->frame = 0;
				if (s->id == ID_SHIP) {
					this->gStatus = LOOSE;
				}
				break;
		}
	}
	if (s->alive) {

		// only enemies need change between frames, unless
		// something explodes
		if (s->id == ID_ENEMY) {
			Nokia5110_PrintBMP(s->x, s->y, s->image[frameCount], 0);
			// ^ frame is always 0, except for enemies
		}
		else {
			Nokia5110_PrintBMP(s->x, s->y, s->image[0], 0);
			// frame is always 0, except for enemies
		}	
	}
	else if (s->jk) {
		Nokia5110_PrintBMP(s->x + offsetX,
						   s->y + offsetY,
						   s->image[this->frame],
						   0);
		this->frame++; // XXX is it a bug that there's no check for wraparound here?
	}
}

// Update position of alive lasers
// inputs: none
// outputs: none
// assumes: na
void GameEngine_laserShipDraw(struct GameEngine *this) {
	unsigned char laserNum;
	for (laserNum=0; laserNum < MAXLASERS; laserNum++) {
		if (this->laser_ship[laserNum].alive) {
			GameEngine_masterDraw(this,
								  &this->laser_ship[laserNum],
								  0);
		}
	}
}

// Send current position to buffer
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_laserEnemyDraw(struct GameEngine *this) {
	unsigned char laserNum;
	for (laserNum=0; laserNum < MAXLASERS; laserNum++) {
		GameEngine_masterDraw(this,
							  &this->laser_enemy[laserNum],
							  0);
	}
}
#endif

#if DRAW_ENEMIES
void
GameEngine_enemyTracking_reset(struct GameEngine *this) {
	this->enemyTracking[0] = FIRST_E;		
	this->enemyTracking[1] = LAST_E;
	this->lowest = FIRST_E;	
	this->highest = LAST_E;
}

// Keep track of the leftmost and rightmost Enemies.
void
GameEngine_enemyShiftTrack(struct GameEngine *this,
						   unsigned int localAliveRows) {
	switch(localAliveRows){
		case 1:
			if (this->gameStatRow[this->lastLine].epr == 1) {
				this->enemyTracking[0] = this->gameStatRow[this->lastLine].fep;
				this->enemyTracking[1] = this->gameStatRow[this->lastLine].fep;
			}
			else {
				this->enemyTracking[1] = this->gameStatRow[this->lastLine].lep;
				this->enemyTracking[0] = this->gameStatRow[this->lastLine].fep;
			}
			break;
		default:{
			signed char row = 0;
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

unsigned long ADC0_In(void){
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
	while (this->gameStatRow[this->lastLine].epr == 0) {
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
// changes: gameStatRow[row].*, gameStatColumn[column].epc, enemyCount, rowAlive[row]
// inputs: row, column, mode
// outputs: none
// assumes: na
unsigned int GameEngine_firstLast(struct GameEngine *this,
								  unsigned int row,
								  unsigned int column) {
	bool lastCheck = false;
	
	this->gameStatRow[row].epr--;
	this->gameStatColumn[column].epc--;
	this->enemyCount--;
	
	if (this->enemyCount == 0) {
		this->rowAlive[row] = false;
		// ^ needed only to update stats before quitting, good for debugging
		GameEngine_setStatus(this, WIN);
	}
	else {
		if (this->gameStatRow[row].epr == 1) {
			lastCheck = true;			//Does forward checking only
		}
		else if (this->gameStatRow[row].epr == 0) {
			this->rowAlive[row] = false;
		}

		if (this->gameStatRow[row].epr) {
			unsigned char column=0;
			bool firstCheck = false;
			for (column=0; column < MAX_ENEMY_PR; column++) {
				if ((! firstCheck) &&
				    (this->enemy[row][column].alive)) {
					// ^ counts forward, check = 0 >> keeps checking
					this->gameStatRow[row].fep = column;
					firstCheck = true;
					if (this->gameStatRow[row].epr == 1) {
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
// changes: gameStatColumn[column].(fep|epc),alColsMat[aliveCol], liveCols
// Callers: EnemyLaserInit, GameEngine_enemyscanX
// outputs: liveCols
// assumes: na
#if DRAW_ENEMIES

void GameEngine_firstEPC_reset(struct GameEngine *this) {
	//sets defaults
	unsigned char i;
	this->liveCols = MAX_ENEMY_PR;
	for (i=0; i < MAX_ENEMY_PR; i++) {
		this->alColsMat[i] = i;
	}
}

void GameEngine_firstEPC(struct GameEngine *this) {
	unsigned char column = 0;
	unsigned char aliveCol = 0;
	
	//we are reading left>right, dowun>up
	for (column=0; column < MAX_ENEMY_PR; column++) {
		signed char row = this->gameStatColumn[column].fep;
		// ^ start from last known position
		assert(row < this->maxrows);
		if (this->gameStatColumn[column].epc == 0) {
			continue;
		}
	
		this->alColsMat[aliveCol] = column;
		aliveCol++;
		
		//finds the first enemy on a column
		while (row>=0) {
			if (this->enemy[row][column].alive) {
				this->gameStatColumn[column].fep = row;
				break;
			}
			else {
				row--;
			}
		}
	}
	this->liveCols = aliveCol;
}
#endif

//----------------------------------Miscelaneus---------------------------------

// Create the enemy bonus ship, and time the reappearence
// changes: na
// inputs: none
// outputs: none
// assumes: na
#if DRAW_BONUSENEMY
void GameEngine_bonusEnemyCreate(struct GameEngine *this) {
	
	if ((this->bonusEnemy.alive == false) &&
		(this->localCounter >= BONUSTIMING)){
		GameEngine_bonusEnemy_init(this);
		this->bonusEnemy.alive = true;
		Sound_Play(&ufoLowPitch);
		this->localCounter = 0;
	}
		
	if (this->bonusEnemy.alive == false) {
		this->localCounter++;
	}
}
#endif

//----------------------------------------------Utilities-----------------------

signed int absValue(signed int value){
	return ((value<0) ? -value:value);
}

// Systick sets the game status on the Engine
// Callers: systick, FirstLast, EnemyLaserCollisions
void GameEngine_setStatus(struct GameEngine *this, const unsigned int v) {
	this->gStatus = v;
}
unsigned int GameEngine_getStatus(struct GameEngine *this) {
	return this->gStatus;
}


#ifdef DEBUG

void GameEngine_pp(struct GameEngine* this, FILE* out) {
	int maxrows= this->maxrows; // XX or ALLOC_MAXROWS ?

	PP_PRINTF("(struct GameEngine) {");
	PP_PRINTF(" .gStatus = %u", this->gStatus);
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
	PP_PRINTF(", .frameCount = %hhu", this->frameCount);
	PP_PRINTF(", .frame = %hhu", this->frame);

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

	PP_PRINTF(", .liveCols = %u", this->liveCols);

	PP_PRINTF(", .alColsMat = { ");
	{
		bool first= true;
		for (int i=0; i< MAX_ENEMY_PR; i++) {
			if (! first) {PP_PRINTF(", ");}
			first = false;
			PP_PRINTF("%u", this->alColsMat[i]);
		}
	}
	PP_PRINTF(" }");

	PP_PRINTF(", .localCounter = %hhu", this->localCounter);

	PP_PRINTF(" }");
}
static void _GameEngine_pp(void* this, FILE* out) { GameEngine_pp(this, out); }

const struct ObjectInterface GameEngine_ObjectInterface = {
	.pp = &_GameEngine_pp
};

#endif


void GameEngine_init(struct GameEngine *this,
					 unsigned int max_number_of_enemy_rows) {
#ifdef DEBUG
	this->vtable = &GameEngine_ObjectInterface;
#endif
    
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
	GameEngine_firstEPC_reset(this);
	GameEngine_enemyLasers_init(this);
#endif
#if DRAW_BONUSENEMY
	GameEngine_bonusEnemy_init(this);
#endif

	// must call GameEngine_shipInit before
	// GameEngine_shipLasersCreation and GameEngine_laserInit_ship2
	GameEngine_shipInit(this);
	GameEngine_shipLasers_init(this);
	//GameEngine_laserInit_ship2(this); XX wrong?

	this->right = true;
	this->down = false;
	this->frameCount = 0;
	this->frame = 0;
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
