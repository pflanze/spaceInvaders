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



//----------------------------------Definitions---------------------------------

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

//Ship collision line

#define SHIPCOLLISIONLINE LOWERLIMIT-PLAYERH+2

//ship midPoints
#define SHIPMIDDLE	((PLAYERW>>1)-1)

//BonusShip
#define BONUS_C_LINE TOPLIMIT+ENEMYBONUSH
#define BONUSTIMING 30
#define OFFSETEXPLOSIONX 2
#define OFFSETEXPLOSIONY 4

//BonusShip midPoint xCoordinate
#define ENEMY_Bon_midX (ENEMYBONUSW>>1)

//enemy midPoint xCoordinate
#define E_LASER_OFFX ((ENEMY30W>>1)-1)
#define E_LASER_OFFY 5

//Objecdt IDs
#define ID_SHIP			0
#define ID_ENEMY		1
#define ID_BONUS		2
#define ID_E_LASER	3
#define ID_S_LASER	4


//Default values
#define	FIRST_E	0
#define	LAST_E	3

//Return vs update (mode)
#define RETURNVAL	0
#define UPDATE		1
#define RETURNARR	3

//Miselaneus
#define NA 1



#if DRAW_ENEMIES
static
void GameEngine_enemyLaserCollisions(struct GameEngine *this);
#endif

//-----------------------------------------------------------INIT---------------
//********GameEngine_enemyInit*****************
//Initialize enemies
//Enemies are counted normaly, top left 00
// changes: Enemy[row|column].*, Estat_row[row].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyInit(struct GameEngine *this) {
	unsigned char row;
	for (row=0; row < this->maxrows; row++) {
		unsigned int column;
		for(column=0;column<MAX_ENEMY_PR;column++){
			this->Enemy[row][column].x = 20*column;
			this->Enemy[row][column].y = 10 + row*10;
			this->Enemy[row][column].life = 1;  // 0=dead, 1=alive
			this->Enemy[row][column].JK = 0;
			this->Enemy[row][column].id = ID_ENEMY;
			switch(row){
				case 0:
					this->Enemy[row][column].image[0] = SmallEnemy30PointA;
					this->Enemy[row][column].image[1] = SmallEnemy30PointB;
					break;
				case 1:
					this->Enemy[row][column].image[0] = SmallEnemy20PointA;
					this->Enemy[row][column].image[1] = SmallEnemy20PointB;
					break;
				case 2:
					this->Enemy[row][column].image[0] = SmallEnemy10PointA;
					this->Enemy[row][column].image[1] = SmallEnemy10PointB;
					break;
			}
		}
		//initializes Estat
		this->Estat_row[row].Epr = MAX_ENEMY_PR;
		// ^ keeps track of the amount of enemies per row
		this->Estat_row[row].Fep = 0;	  // first end position
		this->Estat_row[row].Lep = 3;	  // last end position
	}
}
#endif
//********ShipInit*****************
//It contains the position, image and life of the space ship
// changes: Ship.*
// inputs: none
// outputs: none
// assumes: na
void GameEngine_shipInit(struct GameEngine *this) {
	this->Ship.y = 46;
	this->Ship.image[0] = PlayerShip0;
	this->Ship.life = 1;				// 0=dead, 1=alive
	this->Ship.JK = 0;
	this->Ship.id = ID_SHIP;
}
//********LaserInit_ship*****************
// Function used to initialize the lasers fired by the spaceship
// changes: Laser_ship[index].*
// inputs: none
// outputs: none
// assumes: na
void GameEngine_laserInit_ship(struct GameEngine *this) {
	unsigned char i;
	for(i=0;i<MAXLASERS;i++){
		if(this->Laser_ship[i].life == 0){
			this->Laser_ship[i].x = this->Ship.x + SHIPMIDDLE;
			this->Laser_ship[i].y = 39;
			this->Laser_ship[i].image[0] = Laser0;
			this->Laser_ship[i].life = 1; // 0=dead, 1=alive
			this->Laser_ship[i].id = ID_S_LASER;
			this->Laser_ship[i].JK = 0;
			break; // terminate loop when a slot is found
		}
	}
}

//********LaserInit_ship2*****************
// Function used to initialize the lasers fired by the spaceship
// changes: Laser_ship[index].*
// inputs: none
// outputs: none
// assumes: na
void GameEngine_laserInit_ship2(struct GameEngine *this) {
	unsigned char i;
	unsigned int count = 0;
	
	for(i=0;i<MAXLASERS;i++){
		if(this->Laser_ship[i].life == 0){
			switch(count){
				case 0:
					this->Laser_ship[i].x = this->Ship.x + SHIPMIDDLE;
					count++;
					break; // terminate loop when a slot is found
				case 1:
					this->Laser_ship[i].x = this->Ship.x + 2 + SHIPMIDDLE;
					count++;
					break; // terminate loop when a slot is found
				case 2:	
					this->Laser_ship[i].x = this->Ship.x + 4 + SHIPMIDDLE;
					break; // terminate loop when a slot is found
				
			}
			this->Laser_ship[i].y = 39;
			this->Laser_ship[i].image[0] = Laser0;
			this->Laser_ship[i].life = 1; // 0=dead, 1=alive
			this->Laser_ship[i].id = ID_S_LASER;
			this->Laser_ship[i].JK = 0;
		}
	}
}
//********EnemyLaserInit*****************
//Initializes the lasers fired by the enemy ship. It selects an enemy randomly to shoot.
// changes: Laser_enemy[index].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyLaserInit(struct GameEngine *this) {
	//modifiction may be needed function called twice, please explore
	unsigned int *AliveColsLocal =  GameEngine_firstEPC(this, RETURNVAL);
	unsigned char randN = (Random32()>>24)%(*AliveColsLocal);
	// ^ generates number [0-aliveCols]
	unsigned int *AlColsMatLocal = 	GameEngine_firstEPC(this, RETURNARR);
	unsigned char columnNew	= AlColsMatLocal[randN];
	// ^ matrix holds the valid Enemy firing positions
	
	if (this->Estat_column[columnNew].Epc) {
		unsigned char i;
		for(i=0;i<MAXLASERS;i++){
			if (this->Laser_enemy[i].life == 0) {
				unsigned char row = this->Estat_column[columnNew].Fep;
				this->Laser_enemy[i].x =
					this->Enemy[row][columnNew].x + E_LASER_OFFX;
				this->Laser_enemy[i].y =
					this->Enemy[row][columnNew].y + E_LASER_OFFY;
				this->Laser_enemy[i].image[0] = Laser0;
				this->Laser_enemy[i].life = 1; // 0=dead, 1=alive
				this->Laser_enemy[i].id = ID_E_LASER;
				break; // terminate loop when a slot is found
			}
		}
	}
}
#endif
//********BonusEnemyInit*****************
// Initializes Enemy bonus
// changes: EnemyBonus.*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMYBONUS	
void GameEngine_bonusEnemyInit(struct GameEngine *this) {
	this->EnemyBonus.x = RIGHTLIMIT;
	this->EnemyBonus.y = TOPLIMIT;
	this->EnemyBonus.image[0] = SmallEnemyBonus0;
	this->EnemyBonus.life = 1;
	this->EnemyBonus.JK = 0;
	this->EnemyBonus.id = ID_BONUS;
	
	Sound_Play(&ufoLowPitch);
}
#endif
//-----------------------------------------------------------DEFAULT VALUES-----
//********defaultValues*****************
// Resets the values to default 
// changes: Laser_enemy[i].life, lastLine, Estat_column[i].(Epc|Fep)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_defaultValues(struct GameEngine *this) {
	unsigned char i;
	//tracking defaults
	this->lastLine = this->maxrows - 1;
	//sets defaults column stats
	for(i=0;i<MAX_ENEMY_PR;i++){
		this->Estat_column[i].Epc = this->maxrows;
		this->Estat_column[i].Fep = this->lastLine;
	}	
	for(i=0;i<MAX_ENEMY_PR;i++){
		this->Laser_enemy[i].life = 0;
	}
	
	for(i=0;i<MAXLASERS;i++){
		this->Laser_ship[i].life = 0;
	}
}
#endif
//********reset*****************
// Set values to default using "mode" switch
// changes: Ship.(image|JK), functions: GameEngine_enemyShiftTrack, FirstLast, GameEngine_firstEPC
// Callers: 
// inputs: none
// outputs: none
// assumes: na
//Used by: Function name
void GameEngine_reset(struct GameEngine *this) {
	this->Ship.image[0] = PlayerShip0;
	this->Ship.image[1] = PlayerShip0;
	this->Ship.JK = 0;
#if DRAW_ENEMIES
	GameEngine_enemyShiftTrack(this, NULL, RESET);
	GameEngine_firstEPC(this, RESET);
	GameEngine_firstLast(this, 0, 0, RESET);
#endif
}
//--------------------------------------------------------------MOVE OBJECTS----
//********MoveObjects*****************
//Updates the position values of the asociated objects
// inputs: INGAME
// outputs: none
// assumes: na
void GameEngine_moveObjects(struct GameEngine *this) {
#if DRAW_ENEMIES
	unsigned int *ETracking = GameEngine_enemyShiftTrack(this, NA, RETURNARR);
#endif
	GameEngine_player_move(this); // calls ADC0_in, Convert2Distance
	if (this->gStatus == INGAME) {
#if DRAW_ENEMIES
		GameEngine_enemy_move(this, ETracking[0], ETracking[1]);
		// ^ updates enemy coordinate
		GameEngine_laserEnemy_move(this);
#endif

#if DRAW_ENEMYBONUS
		GameEngine_bonusEnemy_Move(this, INGAME);
#endif
		GameEngine_laserShip_move(this);
	}
}
//********GameEngine_player_move*****************
//Gets the new position value from ADC and updates the spaceShip position
// changes: Ship.x, 
// inputs: none
// outputs: none
// assumes: na
void GameEngine_player_move(struct GameEngine *this) {
	unsigned long ADCdata;
	ADCdata = ADC0_In();
	this->Ship.x = Convert2Distance(ADCdata);
}
//********GameEngine_laserEnemy_move*****************
//updates the enemy coordinates kept in Enemy matrix
// changes: Enemy[row][Column].[x|y]
// inputs: LeftShiftColumn, RightShiftColumn
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemy_move(struct GameEngine *this,
			   unsigned int LeftShiftColumn,
			   unsigned int RightShiftColumn) {
	unsigned char row;
	
	for (row=0; row < this->maxrows; row++) {
		// While not raching the earth
		if (this->Enemy[this->lastLine][0].y < 40) {
			signed char column;
			//sets the switches to move down/left/right
			if (this->Enemy[row][RightShiftColumn].x >= RIGHTLIMIT) {
				this->right = false;	//moves left
				this->down = true;
			}
			else if (this->Enemy[row][LeftShiftColumn].x <= LEFTLIMIT) {
				this->right = true;
				this->down = true;
			}	
			//moves left/right using the switches
			for(column=0;column<MAX_ENEMY_PR;column++){
				if (this->right) {
					this->Enemy[row][column].x += 2; // move to right
				}	
				else {
					this->Enemy[row][column].x -= 2; // move to left
				}	
			}
			//moves down using the switches
			if (this->down) {
				for(column=0;column<4;column++){
					this->Enemy[row][column].y += 1;
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
//********GameEngine_laserShip_move*****************
//Updates the spaceShip'2 laser coordinates
// changes: Laser_ship[index].y, Laser_ship[index].life 
// inputs: none
// outputs: none
// assumes: na
void GameEngine_laserShip_move(struct GameEngine *this) {
 	unsigned char i;
	for(i=0;i<MAXLASERS;i++){
		if (this->Laser_ship[i].life) {
			this->Laser_ship[i].y--;
			
			//laser out of rangef
			if (this->Laser_ship[i].y <= TOPLIMIT) {
				this->Laser_ship[i].life = 0;
				break;
			}
		}
	}
}
//********LaserEnemy_Move*****************
//Updates the laser's friendly ship position 
// changes: Laser_enemy[index].y, Laser_enemy[index].life
// inputs: none
// outputs: none
// assumes: na
//Laser_enemy[i]
#if DRAW_ENEMIES
void GameEngine_laserEnemy_move(struct GameEngine *this) {
 	unsigned char i;
	
	for(i=0;i<MAXLASERS;i++){ // XXX use runtime value not MAXLASERS, RIGHT?
		if (this->Laser_enemy[i].life){
			this->Laser_enemy[i].y++;
			//laser out of range
			if (this->Laser_enemy[i].y > LOWERLIMIT) {//update
				this->Laser_enemy[i].life = 0;
				break;
			}
		}
	}
}
#endif
//********GameEngine_bonusEnemy_Move*****************
// Multiline description
// changes: variablesChanged
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMYBONUS
void GameEngine_bonusEnemy_Move(struct GameEngine *this, unsigned int mode) {
	if (mode == RESET) {
		this->EnemyBonus.life = 0;
		return;
	}
	if (this->EnemyBonus.life) {
		this->EnemyBonus.x--;
		if (this->EnemyBonus.x <= LEFTLIMIT) {
			this->EnemyBonus.life = 0;
		}
	}
}
#endif
//-------------------------------------------DRAWING----------------------------
//********Draw*****************
// Updates the objects positions and sends the data to the screen
// inputs: none
// outputs: none
// assumes: na
void GameEngine_draw(struct GameEngine *this) {
	Nokia5110_ClearBuffer();
	
	//drawing battleship
	GameEngine_masterDraw(this, &(this->Ship), NULL);

	//drawing enemies
#if DRAW_ENEMIES
	GameEngine_enemyDraw(this);			//Uses GameEngine_masterDraw
	GameEngine_laserEnemyDraw(this);		//Uses GameEngine_masterDraw
#endif
	
	//drawing laser
	GameEngine_laserShipDraw(this);		//uses GameEngine_masterDraw

#if DRAW_ENEMYBONUS		
	GameEngine_masterDraw(this, &(this->EnemyBonus), NULL);
#endif	
		
	// draw buffer
	Nokia5110_DisplayBuffer();      
}
//********GameEngine_enemyDraw*****************
//Sends the Enemy information to screen buffer
//changes: Enemy[row][column].image, Enemy[row][column].JK
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyDraw(struct GameEngine *this) {
	signed char row;

	for (row=0; row < this->maxrows; row++) {
		unsigned char column;
		if (this->gStatus == INGAME) { this->FrameCount ^= 0x01; }  // 0,1,0,1,...
		for(column=0;column<4;column++){	
			GameEngine_masterDraw(this,
					      &(this->Enemy[row][column]),
					      this->FrameCount);
		}
	}
}
#endif
//********functionName*****************
// Master function to draw objects
// changes: variablesChanged
// inputs: &EnemyBonus, 
// outputs: none
// assumes: na
void GameEngine_masterDraw(struct GameEngine *this,
			   struct State *s,
			   unsigned int FrameCount) {
	signed char offsetX = 0;
	signed char offsetY = 0;
	
	if(s->JK){
		//used to change explosions offset values
		if(s->id == ID_BONUS){	//BONUS
			Sound_stop_all(&ufoLowPitch);
			Sound_Play(&smallExplosion);
			offsetX = OFFSETEXPLOSIONX;
			offsetY = OFFSETEXPLOSIONY;
		}
		else if(s->id == ID_SHIP){
			Sound_Play(&smallExplosion);
		}
		else if(s->id == ID_E_LASER){
			offsetX = -5;
		}
		else if(s->id == ID_ENEMY){
			Sound_Play(&smallExplosion);
		}
		
		switch (this->frame){
			case 0:
				s->image[0] = SmallExplosion0;
				break;
			case 1:
				s->image[1] = SmallExplosion1;
				break;
			case 2:
				s->JK = 0;
				this->frame = 0;
				if(s->id == ID_SHIP){
					this->gStatus = LOOSE;
				}
				break;
		}
	}
	if(s->life){

		// only enemies need change between frames, unless
		// something explodes
		if (s->id == ID_ENEMY) {
			Nokia5110_PrintBMP(s->x, s->y, s->image[FrameCount], 0);
			// ^ frame is always 0, except for enemies
		}
		else {
			Nokia5110_PrintBMP(s->x, s->y, s->image[0], 0);
			// frame is always 0, except for enemies
		}	
	}
	else if(s->JK){
		Nokia5110_PrintBMP(s->x + offsetX, s->y + offsetY, s->image[this->frame], 0);
		this->frame++; // XXX is it a bug that there's no check for wraparound here?
	}
}
//********GameEngine_laserShipDraw*****************
//Updates position of alive lasers
// inputs: none
// outputs: none
// assumes: na
void GameEngine_laserShipDraw(struct GameEngine *this) {
	unsigned char laserNum;
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
		if (this->Laser_ship[laserNum].life) {
			GameEngine_masterDraw(this,
					      &(this->Laser_ship[laserNum]),
					      NULL);
		}
	}
}
//********GameEngine_laserEnemyDraw*****************
//Send current position to buffer
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_laserEnemyDraw(struct GameEngine *this) {
	unsigned char laserNum;
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
		GameEngine_masterDraw(this,
				      &(this->Laser_enemy[laserNum]),
				      NULL);
	}
}
#endif
//********GameEngine_enemyShiftTrack*****************
//Keeps track if the leftmost and right most Enemies.
#if DRAW_ENEMIES
void
GameEngine_enemyTracking_reset(struct GameEngine *this) {
	this->enemyTracking[0] = FIRST_E;		
	this->enemyTracking[1] = LAST_E;
	this->lowest = FIRST_E;	
	this->highest = LAST_E;
}

unsigned int * GameEngine_enemyShiftTrack(struct GameEngine *this,
					  unsigned int localAliveRows,
					  unsigned int mode) {

	switch(mode){
		case RESET:{
			GameEngine_enemyTracking_reset(this);
			break;
			// XXX is this still necessary as opposed to
			// simply using GameEngine_enemyTracking_reset
			// directly now (i.e. is there magic in
			// continuing to run here?)
		}
		case RETURNARR:
			return this->enemyTracking;
	}
	
	switch(localAliveRows){
		case 1:
			if (this->Estat_row[this->lastLine].Epr == 1) {
				this->enemyTracking[0] = this->Estat_row[this->lastLine].Fep;
				this->enemyTracking[1] = this->Estat_row[this->lastLine].Fep;
			}
			else {
				this->enemyTracking[1] = this->Estat_row[this->lastLine].Lep;
				this->enemyTracking[0] = this->Estat_row[this->lastLine].Fep;
			}
			break;
		default:{
			signed char row = 0;
			this->lowest 	= this->Estat_row[row].Fep;
			this->highest	= this->Estat_row[row].Lep;
			row = 1;
			while (row <= this->maxrows - 1) {//change
				if (this->Estat_row[row].Fep < this->lowest) {
					this->lowest = this->Estat_row[row].Fep;
				}
				if (this->Estat_row[row].Lep > this->highest) {
					this->highest = this->Estat_row[row].Lep;
				}
				row++;
			}
			this->enemyTracking[1] = this->highest;
			this->enemyTracking[0] = this->lowest;
		}	
	}
	return NULL;
}
#endif

//-----------------------------------------------------ADC----------------------

//********Convert2Distance*****************
// Converts the sample value to an equivalent distance
// inputs: sample
// outputs: none
// assumes: na
unsigned long Convert2Distance(unsigned long sample){
	return (Avalue*(sample) >> 10)+Bvalue; // Needs recalibration
}
//********ADC0_In*****************
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
//********Collisions*****************
//Check colision detection: Enemy lasers, ship lasers, laser on laser
// inputs: none
// outputs: none
// assumes: na
// Note: the result pointer must be array to make the collision result homogeneous
void GameEngine_collisions(struct GameEngine *this) {
#if DRAW_ENEMYBONUS
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
//********PlayerLaserCollisions*****************
//Checks if the laser have a hit to an enemy (for every laser)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_playerLaserCollisions(struct GameEngine *this) {
	unsigned char laserNum = 0; 
	//each laser is checked for a collition
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
		if ((this->Laser_ship[laserNum].life) &&
		    (this->Enemy[this->lastLine][0].y + YOFFSET >=
		     this->Laser_ship[laserNum].y)) {
			// found a line with enemies>>start calculating
			// calculate enemy zone(grouping)
			GameEngine_enemyscanY(this, laserNum);
		}	
	}
}
#endif
//********GameEngine_enemyscanY*****************
//scans for an enemy line (y-axis), then sends a request to scan for
//an enemy on the x-axis. Updates the enemy line values
// inputs: lastLine, laserNum
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyscanY(struct GameEngine *this,
			   unsigned int laserNum) {
	signed char row = this->lastLine;
	unsigned char found = 0;
	unsigned char exit = 0;
	
	unsigned char ELL[ALLOC_MAXROWS];		//Enemy line low
	unsigned char ELH[ALLOC_MAXROWS];		//Enemy line high
	
	//it creates an array with thresholds
	for (row= this->lastLine; row >= 0; row--) {
		ELL[row] = this->Enemy[row][0].y + YOFFSET;
		ELH[row] = this->Enemy[row][0].y;
	}
	
	row = this->lastLine;//temporal variable reset
		
	while((found == 0)&&(exit == 0)){
		assert(row >= 0);
		if (this->Laser_ship[laserNum].y > ELL[row]) {
			exit = 1;
		}
		else if (this->Laser_ship[laserNum].y < ELH[row]) {
			row--;
		}
		else {
			GameEngine_enemyscanX(this, row, laserNum);
			found = 1;
		}
		exit = 1;
	}
}	
#endif
//********Verify_lastLine*****************
//Keeps track of the last enemy line
// changes: lastLine
// inputs: none
// outputs: lastLine
// assumes: na
#if DRAW_ENEMIES
static
void GameEngine_update_lastLine(struct GameEngine *this) {
	while(this->Estat_row[this->lastLine].Epr == 0){
		if (this->lastLine == 0)
			break; // correct?
		this->lastLine--;
	}
}
#endif
//********GameEngine_enemyscanX*****************
// Scans for a enemy collition on a single row (x axis)
// changes: Laser_ship[index].life, Enemy[row][column].life, Enemy[row][column].JK
// inputs: row, laserNum
// outputs: enemyDestroyed(0:1)
// assumes: na
#if DRAW_ENEMIES
void GameEngine_enemyscanX(struct GameEngine *this,
			   unsigned int row,
			   unsigned int laserNum) {
	unsigned char column; 
	
	for (column=0; column <= this->Estat_row[row].Lep; column++) {
		if(this->Enemy[row][column].life){
			//checking x coordinate of each active laser against each enemy
			signed char enemyInRange =
				(this->Enemy[row][column].x
				 + E_LASER_OFFX
				 - this->Laser_ship[laserNum].x);
			enemyInRange = absValue(enemyInRange);
			if (enemyInRange <= E_LASER_OFFX){	
				unsigned int alive_rows;
				this->Laser_ship[laserNum].life = 0;
				this->Enemy[row][column].life = 0;
				this->Enemy[row][column].JK = 1;
				alive_rows = GameEngine_firstLast(this, row, column, UPDATE);
				GameEngine_update_lastLine(this);
				//updates 
				GameEngine_enemyShiftTrack(this, alive_rows, UPDATE);
				GameEngine_firstEPC(this, UPDATE); //update point
				break; //return????
			}
		}
	}	
}
#endif
//********EnemyLaserCollisions*****************
// Detects the collision between laserEnemy and our ship
// changes: Ship.life, Ship.JK
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
static
void GameEngine_enemyLaserCollisions(struct GameEngine *this) {
	if(this->Ship.life){
		unsigned char i = 0;
		for(i=0;i<MAXLASERS;i++){
			//check agains the ship
			if (this->Laser_enemy[i].life &&
			    (this->Laser_enemy[i].y > SHIPCOLLISIONLINE)){
				signed char collision = this->Ship.x
					+ SHIPMIDDLE
					- this->Laser_enemy[i].x;
				collision = absValue(collision);
				if(collision <= SHIPMIDDLE){
					this->Laser_enemy[i].life = 0;
					this->Laser_enemy[i].JK = 0;
					this->Ship.life = 0;
					this->Ship.JK = 1;
				}
			}
		}
	}
}
#endif
//********LaserCollision*****************
// Detects collisions between lasers
// changes: variablesChanged
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void GameEngine_laserCollision(struct GameEngine *this) {
	unsigned char lasernumEnemy;
	//checks collision course
	for(lasernumEnemy=0;lasernumEnemy<MAXLASERS;lasernumEnemy++){
		if (this->Laser_enemy[lasernumEnemy].life) {
			unsigned char lasernumShip = 0; //avoids unnecessary comparison
			for(lasernumShip=0;lasernumShip<MAXLASERS;lasernumShip++){
				// avoid unnecessary comparison
				if (this->Laser_ship[lasernumShip].life) {
					signed char xDistance =
						this->Laser_enemy[lasernumEnemy].x
						- this->Laser_ship[lasernumShip].x;
					if ((absValue(xDistance)<2) &&
					    (this->Laser_ship[lasernumShip].life)) {
						signed char yDistance =
							(this->Laser_enemy[lasernumEnemy].y
							 + LASERH)
							> this->Laser_ship[lasernumShip].y;
						// ^ crossOver each other
						if(yDistance){
							this->Laser_enemy[lasernumEnemy].life = 0;
							this->Laser_enemy[lasernumEnemy].JK = 1;
							this->Laser_ship[lasernumShip].life = 0;
						}
					}
				}
			}
		}
	}
}
#endif
//********BonusLaserCollision*****************
// Detects collision between shipLaser and EnemyBonus
// changes: EnemyBonus (JK, life), Laser_ship (JK, life)
// inputs: none
// outputs: none
// assumes: na
//notes: BonusLaserCollision does not need return (game does not terminate)
#if DRAW_ENEMYBONUS
void GameEngine_bonusLaserCollision(struct GameEngine *this) {
	if (this->EnemyBonus.life) {
		unsigned char laserNumber = 0;
		for(laserNumber=0;laserNumber<MAXLASERS;laserNumber++){
			if (this->Laser_ship[laserNumber].life) {
				if (this->Laser_ship[laserNumber].y <= BONUS_C_LINE) {
					// any of the lasers passes certain threshold
					signed char xDistance =
						this->EnemyBonus.x
						+ ENEMY_Bon_midX
						- this->Laser_ship[laserNumber].x;
					xDistance = absValue(xDistance);
					if(xDistance < ENEMY_Bon_midX){
						this->EnemyBonus.life = 0;
						this->EnemyBonus.JK = 1;
						this->Laser_ship[laserNumber].life = 0;
						this->Laser_ship[laserNumber].JK = 1;
					}
				}
			}
		}
	}
}
#endif
//-----------------------------------------GAME STATS---------------------------
//********FirstLast*****************
//this function should keep track of:
//					the number of enemies on each row
//					It is called by GameEngine_enemyscanX to update Estat_row
//					it is also used to update general game stats
// changes: Estat_row[row].*, Estat_column[column].Epc, enemyCount, AliveRows[row]
// inputs: row, column, mode
// outputs: none
// assumes: na
#if DRAW_ENEMIES
static unsigned char enemyCount; // XXXX move to local scope or to this ?
unsigned int GameEngine_firstLast(struct GameEngine *this,
				  unsigned int row,
				  unsigned int column,
				  unsigned int mode) {
	unsigned char lastCheck = 0;
	
	//setting defaults
	if(mode == RESET) {
		enemyCount = this->maxrows * MAX_ENEMY_PR; // see COPYPASTE
		{	//liverows[] defaults
			unsigned char i;
			for (i=0; i < this->maxrows; i++) {
				this->AliveRows[i] = 1;
			}
		}
		return 0;
	}
	
	this->Estat_row[row].Epr--;
	this->Estat_column[column].Epc--;
	enemyCount--;
	
	if(enemyCount == 0){
		this->AliveRows[row] = 0;
		// ^ needed only to update stats before quiting, good for debugging
		GameEngine_setStatus(this, WIN);
	}
	else{
		if (this->Estat_row[row].Epr == 1) {
			lastCheck = 1;			//Does forward checking only
		}
		else if (this->Estat_row[row].Epr == 0) {
			this->AliveRows[row] = 0;
		}

		if (this->Estat_row[row].Epr) {
			unsigned char column=0;
			unsigned char firstCheck = 0;
			for(column=0;column<MAX_ENEMY_PR;column++){
				if ((firstCheck == 0) &&
				    (this->Enemy[row][column].life)) {
					// ^ counts forward, check = 0 >> keeps checking
					this->Estat_row[row].Fep = column;
					firstCheck = 1;
					if (this->Estat_row[row].Epr == 1) {
						this->Estat_row[row].Lep =
							this->Estat_row[row].Fep;
					}
				}
				
				if ((lastCheck == 0) &&
				    (this->Enemy[row][REAL_MAX_EPR-column].life)) {
					// ^ counts backwards
					this->Estat_row[row].Lep = REAL_MAX_EPR-column;
					lastCheck = 1;
				}
				
				if((firstCheck)&&(lastCheck)){
					break;
				}
			}
		}
	}

	{unsigned int alr_counter=0;
		{unsigned char i;
			for(i=0; i <= this->maxrows - 1; i++) {
				if (this->AliveRows[i]) {
					alr_counter++;
				}
			}
		}
		return alr_counter;
	}
}
#endif
//********GameEngine_firstEPC*****************
// Keep track of the first enemy per column
// changes: Estat_column[column].(Fep|Epc),AlColsMat[aliveCol], LiveCols
// Callers: EnemyLaserInit, GameEngine_enemyscanX
// inputs: mode = RETURNVAL|UPDATE|RETURNARR|RESET
// outputs: LiveCols
// assumes: na
#if DRAW_ENEMIES
unsigned int * GameEngine_firstEPC(struct GameEngine *this, unsigned int mode) {
	static unsigned int LiveCols = MAX_ENEMY_PR;
	static unsigned int AlColsMat[MAX_ENEMY_PR] = {0,1,2,3};
	unsigned char column = 0;
	unsigned char aliveCol = 0;
	
	//sets defaults
	switch(mode){
		case RESET:{
			unsigned int *p = 0;
			unsigned char i;
			LiveCols = MAX_ENEMY_PR;
			for(i=0;i<MAX_ENEMY_PR;i++){
				AlColsMat[i] = i;
			}
			return p;
		}
		//RETURNVAL return the alive columns
		case RETURNVAL:{
			unsigned int *p = &LiveCols;
			return p;
		}	
	}
	
	//RETURNARR will continue
	//we are reading left>right, dowun>up
	for(column=0;column<MAX_ENEMY_PR;column++){
		signed char row = this->Estat_column[column].Fep;
		// ^ start from last known position
		if (this->Estat_column[column].Epc == 0) {
			continue;
		}
	
		AlColsMat[aliveCol] = column;
		aliveCol++;
		
		//finds the first enemy on a column
		while(row>=0){
			if (this->Enemy[row][column].life) {
				this->Estat_column[column].Fep = row;
				break;
			}
			else {
				row--;
			}
		}
	}
	LiveCols = aliveCol;
	{//RETURNARR mode return the array
		unsigned int *p = AlColsMat;
		return p;
	}
}
#endif
//----------------------------------Miscelaneus---------------------------------
//********enemyBonusCreate*****************
// It creates the Enemy bonus ship, and times the reApereance
// changes: na
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMYBONUS
void GameEngine_enemyBonusCreate(struct GameEngine *this) {
	static unsigned char localCounter = 0;
	
	if ((this->EnemyBonus.life == 0) && (localCounter >= BONUSTIMING)){
		GameEngine_bonusEnemyInit(this);
		localCounter = 0;
	}
		
	if (this->EnemyBonus.life == 0) {
		localCounter++;
	}
}
#endif
//----------------------------------------------Utilities-----------------------
//********absValue*****************
// Returns positive value
// changes: value
// inputs: value
// outputs: value
// assumes: na
signed int absValue(signed int value){
	return ((value<0) ? -value:value);
}
//********GameEngine_setStatus*****************
// Systick sets the game status on the Engine
// changes: na
// Callers: systick, FirstLast, EnemyLaserCollisions
// inputs: none
// outputs: none
// assumes: na
void GameEngine_setStatus(struct GameEngine *this, const unsigned int v){
	this->gStatus = v;
}
//********getStatus*****************
// Returns the current game status
// changes: na
// Callers: systick
// inputs: none
// outputs: none
// assumes: na
unsigned int GameEngine_getStatus(struct GameEngine *this) {
	return this->gStatus;
}



void GameEngine_init(struct GameEngine *this,
		     unsigned int max_number_of_enemy_rows) {
	this->gStatus = STANDBY;
	assert(max_number_of_enemy_rows <= ALLOC_MAXROWS);
	this->maxrows= max_number_of_enemy_rows;
	this->lastLine= this->maxrows - 1;
	enemyCount= this->maxrows * MAX_ENEMY_PR; // COPYPASTE
	this->right = true;
	this->down = false;
	this->FrameCount = 0;
	this->frame = 0;
	GameEngine_enemyTracking_reset(this);
	this->lowest = FIRST_E;
	this->highest = LAST_E;
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
