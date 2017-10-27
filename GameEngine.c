//GameEngine.c
/*
The game engine keeps track of game statistics to perform movement, drawing, shooting.
In general terms the statistics tracked are: enemies per row/column, first/last enemy per row, first enemy per column, total number of enemies,
rows/columns with enemies alive,
*/

#include "GameEngine.h"
#include "sprites.h"
#include "Nokia5110.h"
#include "..//tm4c123gh6pm.h"
#include "Random.h"
#include "utils.h"

//local variables
//static unsigned gStatus = STANDBY;
volatile static unsigned int gStatus = STANDBY;

#if DRAW_ENEMIES
	//use to keep the statistics from the game
	static unsigned lastLine = MAXROWS-1;
#endif

//----------------------------------------------------------Structs------------------------------------------------
//game stats per column
#if DRAW_ENEMIES
struct GameStatColumn {
	unsigned char Fep;		//"First enemy position"
	unsigned char Epc;		//"Enemies per column"
};

static struct GameStatColumn Estat_column[MAX_ENEMY_PR];

//game stats per row
struct GameStatRow {
	unsigned char Fep;		//"First enemy position"
	unsigned char Lep;		//"Last enemy position"
	unsigned char Epr;		//"Enemies per row"
};

static struct GameStatRow Estat_row[MAXROWS];
static struct State Enemy[MAXROWS][MAX_ENEMY_PR];
static struct State Laser_enemy[MAXLASERS];
static struct State Ship;
static struct State Laser_ship[MAXLASERS];
#endif

#if DRAW_ENEMYBONUS
	static struct State  EnemyBonus;
#endif	

//-----------------------------------------------------------INIT-----------------------------------------------------------------------
//********EnemyInit*****************
//Initialize enemies
//Enemies are counted normaly, top left 00
// changes: Enemy[row|column].*, Estat_row[row].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void EnemyInit(void){
	unsigned char row;
	for(row=0;row<MAXROWS;row++){
		unsigned int column;
		for(column=0;column<MAX_ENEMY_PR;column++){
			Enemy[row][column].x = 20*column;
			Enemy[row][column].y = 10 + row*10;
			Enemy[row][column].life = 1;				// 0=dead, 1=alive
			Enemy[row][column].JK = 0;
			Enemy[row][column].id = ID_ENEMY;
			switch(row){
				case 0:
					Enemy[row][column].image[0] = SmallEnemy30PointA;
					Enemy[row][column].image[1] = SmallEnemy30PointB;
					break;
				case 1:
					Enemy[row][column].image[0] = SmallEnemy20PointA;
					Enemy[row][column].image[1] = SmallEnemy20PointB;					
					break;
				case 2:
					Enemy[row][column].image[0] = SmallEnemy10PointA;
					Enemy[row][column].image[1] = SmallEnemy10PointB;
					break;
			}
		}
		//initializes Estat
		Estat_row[row].Epr = MAX_ENEMY_PR;	//keeps track od the amount of enemies per row
		Estat_row[row].Fep = 0;							//first end position
		Estat_row[row].Lep = 3;							//last end position
	}
}
#endif
//********ShipInit*****************
//It contains the position, image and life of the space ship
// changes: Ship.*
// inputs: none
// outputs: none
// assumes: na
void ShipInit(void){ 
	Ship.y = 46;
	Ship.image[0] = PlayerShip0;
	Ship.life = 1;				// 0=dead, 1=alive
	Ship.JK = 0;
	Ship.id = ID_SHIP;
}
//********LaserInit_ship*****************
// Function used to initialize the lasers fired by the spaceship
// changes: Laser_ship[index].*
// inputs: none
// outputs: none
// assumes: na
void LaserInit_ship(void){
	unsigned char i;
	for(i=0;i<MAXLASERS;i++){
		if(Laser_ship[i].life == 0){
			Laser_ship[i].x = Ship.x + SHIPMIDDLE;
			Laser_ship[i].y = 39;
			Laser_ship[i].image[0] = Laser0;
			Laser_ship[i].life = 1;				// 0=dead, 1=alive
			Laser_ship[i].id = ID_S_LASER;
			Laser_ship[i].JK = 0;
			break;			//terminate loop when a slot is found
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
	void EnemyLaserInit(void){	
		//modifiction may be needed function called twice, please explore
		unsigned int *AliveColsLocal =  FirstEPC(RETURNVAL);
		unsigned char randN = (Random32()>>24)%(*AliveColsLocal);		//generates number [0-aliveCols]
		unsigned int *AlColsMatLocal = 	FirstEPC(RETURNARR);
		unsigned char columnNew	= AlColsMatLocal[randN];					//matrix holds the valid Enemy firing positions
		
		if(Estat_column[columnNew].Epc){
			unsigned char i;
			for(i=0;i<MAXLASERS;i++){
				if(Laser_enemy[i].life == 0){
					unsigned char row = Estat_column[columnNew].Fep;
					Laser_enemy[i].x = Enemy[row][columnNew].x + E_LASER_OFFX;
					Laser_enemy[i].y = Enemy[row][columnNew].y + E_LASER_OFFY;
					Laser_enemy[i].image[0] = Laser0;
					Laser_enemy[i].life = 1;				//0=dead, 1=alive
					Laser_enemy[i].id = ID_E_LASER;
					break;			//terminate loop when a slot is found
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
void BonusEnemyInit(void){
	EnemyBonus.x = RIGHTLIMIT;
	EnemyBonus.y = TOPLIMIT;
	EnemyBonus.image[0] = SmallEnemyBonus0;
	EnemyBonus.life = 1;
	EnemyBonus.JK = 0;
	EnemyBonus.id = ID_BONUS;
}
#endif
//-----------------------------------------------------------DEFAULT VALUES-----------------------------------------------------------------------
//********defaultValues*****************
// Resets the values to default 
// changes: Laser_enemy[i].life, lastLine, Estat_column[i].(Epc|Fep)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void defaultValues(void){
	unsigned char i;
	//tracking defaults
	lastLine = MAXROWS-1;
	//sets defaults column stats
	for(i=0;i<MAX_ENEMY_PR;i++){
		Estat_column[i].Epc = MAXROWS;
		Estat_column[i].Fep = lastLine;
	}	
	for(i=0;i<MAXLASERS;i++){											
		Laser_enemy[i].life = 0;
	}
}
#endif
//********reset*****************
// Set values to default using "mode" switch
// changes: Ship.(image|JK), functions: EnemyShiftTrack, FirstLast, FirstEPC
// Callers: 
// inputs: none
// outputs: none
// assumes: na
//Used by: Function name
void reset(void){
	Ship.image[0] = PlayerShip0;
	Ship.image[1] = PlayerShip0;
	Ship.JK = 0;
	EnemyShiftTrack(NULL, RESET);
	FirstLast(NULL, NULL, RESET);
	FirstEPC(RESET);
}
//--------------------------------------------------------------MOVE OBJECTS-------------------------------------------------------------------------
//********MoveObjects*****************
//Updates the position values of the asociated objects
// inputs: INGAME
// outputs: none
// assumes: na
void MoveObjects(void){
	unsigned int *ETracking = EnemyShiftTrack(NA,RETURNARR);
	Player_Move();						//calls ADC0_in, Convert2Distance
	if(gStatus == INGAME){
#if DRAW_ENEMIES
	Enemy_Move(ETracking[0], ETracking[1]);					//updates enemy coordinate
	LaserEnemy_Move();
#endif

#if DRAW_ENEMYBONUS
	BonusEnemy_Move(INGAME);
#endif
		LaserShip_Move();
	}
}
//********Player_Move*****************
//Gets the new position value from ADC and updates the spaceShip position
// changes: Ship.x, 
// inputs: none
// outputs: none
// assumes: na
void Player_Move(void){
	unsigned long ADCdata;
	ADCdata = ADC0_In();
	Ship.x = Convert2Distance(ADCdata);
}
//********Enemy_Move*****************
//updates the enemy coordinates kept in Enemy matrix
// changes: Enemy[row][Column].[x|y]
// inputs: LeftShiftColumn, RightShiftColumn
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void Enemy_Move(unsigned char LeftShiftColumn, unsigned char RightShiftColumn){ 
	unsigned char row = 0;
	
	while(row<MAXROWS){
		if(Enemy[lastLine][0].y < 40){			//Do it while not raching the earth, At 40 the ships have reach the earth!
			signed char column;
			static unsigned char right = TRUE;			//moves the enemies, 0: moves left
			static unsigned char down = FALSE;			//moves the enemies, 1: moves down
			//sets the switches to move down/left/right
			if(Enemy[row][RightShiftColumn].x >= RIGHTLIMIT){
				right = FALSE;	//moves left
				down = TRUE;
			}
			else if(Enemy[row][LeftShiftColumn].x <= LEFTLIMIT){
				right = TRUE;
				down = TRUE;
			}	
			//moves left/right using the switches
			for(column=0;column<MAX_ENEMY_PR;column++){
				if(right){
					Enemy[row][column].x += 2; // move to right
				}	
				else{
					Enemy[row][column].x -= 2; // move to left
				}	
			}
			//moves down using the switches
			if(down){
				for(column=0;column<4;column++){
					Enemy[row][column].y += 1;
				}
				down = FALSE;
			}
		}
		else{										//Enemies have reached the earth
			gStatus = LOOSE;
		}
		row++;
	}
}
#endif
//********LaserShip_Move*****************
//Updates the spaceShip'2 laser coordinates
// changes: Laser_ship[index].y, Laser_ship[index].life 
// inputs: none
// outputs: none
// assumes: na
void LaserShip_Move(void){
 	unsigned char i;
	for(i=0;i<MAXLASERS;i++){
		if(Laser_ship[i].life){
			Laser_ship[i].y--;
			
			//laser out of rangef
			if(Laser_ship[i].y <= TOPLIMIT){
				Laser_ship[i].life = 0;
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
void LaserEnemy_Move(void){
 	unsigned char i;
	
	for(i=0;i<MAXLASERS;i++){
		if (Laser_enemy[i].life){
			Laser_enemy[i].y++;
			//laser out of range
			if(Laser_enemy[i].y > LOWERLIMIT){//update
				Laser_enemy[i].life = 0;
				break;
			}
		}
	}
}
#endif
//********BonusEnemy_Move*****************
// Multiline description
// changes: variablesChanged
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMYBONUS
void BonusEnemy_Move(unsigned int mode){
	if(mode == RESET){EnemyBonus.life = 0;return;}
	if(EnemyBonus.life){
		EnemyBonus.x--;
		if(EnemyBonus.x <= LEFTLIMIT){
			EnemyBonus.life = 0;
		}
	}
}
#endif
//------------------------------------------------------------------------DRAWING----------------------------------------------------------------------
//********Draw*****************
// Updates the objects positions and sends the data to the screen
// inputs: none
// outputs: none
// assumes: na
void Draw(void){ 
	Nokia5110_ClearBuffer();
	
	//drawing battleship
	MasterDraw(&Ship, NULL);

	//drawing enemies
#if DRAW_ENEMIES
	EnemyDraw();			//Uses MasterDraw
	LaserEnemyDraw();		//Uses MasterDraw
#endif
	
//drawing laser
LaserShipDraw();		//uses MasterDraw

#if DRAW_ENEMYBONUS		
	MasterDraw(&EnemyBonus, NULL);
#endif	
		
	// draw buffer
	Nokia5110_DisplayBuffer();      
}
//********EnemyDraw*****************
//Sends the Enemy information to screen buffer
//changes: Enemy[row][column].image, Enemy[row][column].JK
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void EnemyDraw(void){
	signed char row=0;

	while(row<MAXROWS){
		unsigned char column;
		static unsigned char FrameCount = 0;
		if(gStatus == INGAME){FrameCount = (FrameCount+1)&0x01;}	// 0,1,0,1,...
		for(column=0;column<4;column++){	
			MasterDraw(&Enemy[row][column], FrameCount);
		}
		row++;
	}
}
#endif
//********functionName*****************
// Master function to draw objects
// changes: variablesChanged
// inputs: &EnemyBonus, 
// outputs: none
// assumes: na
void MasterDraw(struct State *s, unsigned char FrameCount){
	static unsigned char frame = 0;
	signed char offsetX = 0;
	signed char offsetY = 0;
	
	if(s->JK){
		//used to change explosions offset values
		if(s->id == ID_BONUS){	//BONUS
			offsetX = OFFSETEXPLOSIONX;
			offsetY = OFFSETEXPLOSIONY;
		}
		else if(s->id == ID_E_LASER){	//BONUS
			offsetX = -5;
		}
		
		switch (frame){
			case 0:
				s->image[0] = SmallExplosion0;
				break;
			case 1:
				s->image[1] = SmallExplosion1;
				break;
			case 2:
				s->JK = 0;
				frame = 0;
				if(s->id == ID_SHIP){
					gStatus = LOOSE;
				}
				break;
		}
	}
	if(s->life){

		if(s->id == ID_ENEMY){		//only enemies need change between frames, unless something explodes
			Nokia5110_PrintBMP(s->x, s->y, s->image[FrameCount], 0); //frame is always 0, except for enemies
		}
		else{
			Nokia5110_PrintBMP(s->x, s->y, s->image[0], 0); //frame is always 0, except for enemies
		}	
	}
	else if(s->JK){
		Nokia5110_PrintBMP(s->x + offsetX, s->y + offsetY, s->image[frame], 0);
		frame++;
	}
}
//********LaserShipDraw*****************
//Updates position of alive lasers
// inputs: none
// outputs: none
// assumes: na
void LaserShipDraw(void){
	unsigned char laserNum;
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
		if(Laser_ship[laserNum].life){
			MasterDraw(&Laser_ship[laserNum], NULL);
		}
	}
}
//********LaserEnemyDraw*****************
//Send current position to buffer
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void LaserEnemyDraw(void){
	unsigned char laserNum;
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
				MasterDraw(&Laser_enemy[laserNum], NULL);
	}
}
#endif
//********EnemyShiftTrack*****************
//Keeps track if the leftmost and right most Enemies.
// changes: enemyTracking[]
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
unsigned int * EnemyShiftTrack(unsigned int localAliveRows, unsigned char mode){
	static unsigned int enemyTracking[] = {FIRST_E,LAST_E};	//keeps track of the first and last enemy across diferrent rows
	static unsigned char lowest = FIRST_E;		//represents the lowest column number with a enemy alive (general)
	static unsigned char highest = LAST_E;		//represents the higest column number with a enemy alive (general)
	
	switch(mode){
		case RESET:{
			enemyTracking[0] = FIRST_E;		
			enemyTracking[1] = LAST_E;
			lowest = FIRST_E;	
			highest = LAST_E;
			break;
		}	
		case RETURNARR:
			return enemyTracking;
	}
	
	switch(localAliveRows){
		case 1:
			if(Estat_row[lastLine].Epr == 1){
				enemyTracking[0] = Estat_row[lastLine].Fep;
				enemyTracking[1] = Estat_row[lastLine].Fep;
			}
			else{
				enemyTracking[1] = Estat_row[lastLine].Lep;
				enemyTracking[0] = Estat_row[lastLine].Fep;
			}
			break;
		default:{
			signed char row = 0;
			lowest 	= Estat_row[row].Fep;
			highest	= Estat_row[row].Lep;
			row = 1;
			while(row <= MAXROWS-1){
				if(Estat_row[row].Fep < lowest){
					lowest = Estat_row[row].Fep;
				}
				if(Estat_row[row].Lep > highest){
					highest = Estat_row[row].Lep;
				}
				row++;
			}
			enemyTracking[1] = highest;
			enemyTracking[0] = lowest;
		}	
	}
	return NULL;
}
#endif

//-------------------------------------------------------------------------ADC--------------------------------------------------

//********Convert2Distance*****************
// Converts the sample value to an equivalent distance
// inputs: sample
// outputs: none
// assumes: na
unsigned long Convert2Distance(unsigned long sample){
	return (Avalue*(sample) >> 10)+Bvalue; 					//Needs recalibration
}
//********ADC0_In*****************
// ADC converter
// inputs: none
// outputs: result (of the conversion)
// assumes: na
unsigned long ADC0_In(void){  
  unsigned long result;
  ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;
}
//------------------------------------------------------------------COLLISIONS------------------------------------------------
//********Collisions*****************
//Check colision detection: Enemy lasers, ship lasers, laser on laser
// inputs: none
// outputs: none
// assumes: na
// Note: the result pointer must be array to make the collision result homogeneous
void Collisions(void){
#if DRAW_ENEMYBONUS
	BonusLaserCollision();			//does not retuen value
#endif
	
#if DRAW_ENEMIES
#if	GODMODE
	EnemyLaserCollisions();
#endif	
	PlayerLaserCollisions();
	LaserCollision();
#endif
}
//********PlayerLaserCollisions*****************
//Checks if the laser have a hit to an enemy (for every laser)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void PlayerLaserCollisions(void){
	unsigned char laserNum = 0; 
		//each laser is checked for a collition
		for(laserNum=0;laserNum<MAXLASERS;laserNum++){
			if((Laser_ship[laserNum].life)&&(Enemy[lastLine][0].y + YOFFSET >= Laser_ship[laserNum].y)){//found a line with enemies>>start calculating
				//calculate enemy zone(grouping)
				EnemyscanY(laserNum);
			}	
		}
}
#endif
//********EnemyscanY*****************
//scans for an enemy line (y-axis), then sends a request to scan for an enemy on the x-axis. Updates the enemy line values
// inputs: lastLine, laserNum
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void EnemyscanY(unsigned char laserNum){
	signed char row = lastLine;
	unsigned char found = 0;
	unsigned char exit = 0;
	
	unsigned char ELL[MAXROWS];		//Enemy line low
	unsigned char ELH[MAXROWS];		//Enemy line high
	
	//it creates an array with thresholds
	for(row=lastLine;row>=0;row--){
		ELL[row] = Enemy[row][0].y + YOFFSET;
		ELH[row] = Enemy[row][0].y;
	}
	
	row = lastLine;//temporal variable reset
		
	while((found == 0)&&(exit == 0)){
		if(Laser_ship[laserNum].y > ELL[row]){
			exit = 1;
		}
		else if(Laser_ship[laserNum].y < ELH[row]){
		 row--;
		}
		else{
			EnemyscanX(row, laserNum);
			found = 1;
		}
		exit = 1;
	}
}	
#endif
//********EnemyscanX*****************
// Scans for a enemy collition on a single row (x axis)
// changes: Laser_ship[index].life, Enemy[row][column].life, Enemy[row][column].JK
// inputs: row, laserNum
// outputs: enemyDestroyed(0:1)
// assumes: na
#if DRAW_ENEMIES
void EnemyscanX(unsigned char row, unsigned char laserNum){
	unsigned char column = 0; 
	unsigned char enemyDestroyed = 0;
	
	while((enemyDestroyed == 0)&&(column <= Estat_row[row].Lep)){
		if(Enemy[row][column].life){
			//checking x coordinate of each active laser against each enemy
			signed char enemyInRange = (Enemy[row][column].x + E_LASER_OFFX - Laser_ship[laserNum].x);
			enemyInRange = absValue(enemyInRange);
			if (enemyInRange <= E_LASER_OFFX){	
				unsigned int *alive_rows;
				Laser_ship[laserNum].life = 0;
				Enemy[row][column].life = 0;
				Enemy[row][column].JK = 1;
				alive_rows = FirstLast(row, column, UPDATE);
				lastLine = Verify_lastLine(lastLine);								//updates last line value
#if DRAW_ENEMIES
					//updates 
					EnemyShiftTrack(*alive_rows, UPDATE);
#endif
				FirstEPC(UPDATE);																					//update point
				enemyDestroyed = 1;
			}
		}
		column++;
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
void EnemyLaserCollisions(void){
	if(Ship.life){
		unsigned char i = 0;
		for(i=0;i<MAXLASERS;i++){
			//check agains the ship
			if(Laser_enemy[i].life && (Laser_enemy[i].y > SHIPCOLLISIONLINE)){
				signed char collision = ( Ship.x + SHIPMIDDLE - Laser_enemy[i].x);
				collision = absValue(collision);
				if(collision <= SHIPMIDDLE){
					Laser_enemy[i].life = 0;
					Laser_enemy[i].JK = 0;
					Ship.life = 0;
					Ship.JK = 1;
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
void LaserCollision(void){
	unsigned char lasernumEnemy;
	//checks collision course
	for(lasernumEnemy=0;lasernumEnemy<MAXLASERS;lasernumEnemy++){
		if(Laser_enemy[lasernumEnemy].life){
			unsigned char lasernumShip = 0;			//avoids unnecessary comparison
			for(lasernumShip=0;lasernumShip<MAXLASERS;lasernumShip++){
				if(Laser_ship[lasernumShip].life){																									//avoids unnecessary comparison
					signed char xDistance = (Laser_enemy[lasernumEnemy].x - Laser_ship[lasernumShip].x);
					if((absValue(xDistance)<2)&&(Laser_ship[lasernumShip].life)){
						signed char yDistance = ((Laser_enemy[lasernumEnemy].y+LASERH) > Laser_ship[lasernumShip].y);	//crossOver each other
						if(yDistance){
							Laser_enemy[lasernumEnemy].life = 0;
							Laser_enemy[lasernumEnemy].JK = 1;
							Laser_ship[lasernumShip].life = 0;
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
void BonusLaserCollision(void){
	if(EnemyBonus.life){
		unsigned char laserNumber = 0;
		for(laserNumber=0;laserNumber<MAXLASERS;laserNumber++){
			if(Laser_ship[laserNumber].life){
				if(Laser_ship[laserNumber].y <= BONUS_C_LINE){			//if any of the lasers pases certain threshold
					signed char xDistance = (EnemyBonus.x + ENEMY_Bon_midX - Laser_ship[laserNumber].x);
					xDistance = absValue(xDistance);
					if(xDistance < ENEMY_Bon_midX){
						EnemyBonus.life = 0;
						EnemyBonus.JK = 1;
						Laser_ship[laserNumber].life = 0;
						Laser_ship[laserNumber].JK = 1;
					}
				}
			}
		}
	}
}
#endif
//--------------------------------------------------------------GAME STATS----------------------------------------------------------
//********FirstLast*****************
//this function should keep track of:
//					the number of enemies on each row
//					It is called by EnemyscanX to update Estat_row
//					it is also used to update general game stats
// changes: Estat_row[row].*, Estat_column[column].Epc, enemyCount, AliveRows[row]
// inputs: row, column, mode
// outputs: none
// assumes: na
#if DRAW_ENEMIES
unsigned int * FirstLast(unsigned char row, unsigned char column, unsigned char mode){
	unsigned char lastCheck = 0;
	static unsigned char enemyCount = MAXROWS * MAX_ENEMY_PR;
	static unsigned char AliveRows[MAXROWS];
	
	//setting defaults
	if(mode == RESET) {
		enemyCount = MAXROWS * MAX_ENEMY_PR;
		{	//liverows[] defaults
			unsigned char i;
			for(i=0;i<MAXROWS;i++){
				AliveRows[i] = 1;
			}
		}
		return NULL;
	}
	
	Estat_row[row].Epr--;
	Estat_column[column].Epc--;
	enemyCount--;
	
	if(enemyCount == 0){
		AliveRows[row] = 0;					//needed only to update stats before quiting, good for debugging
		gStatus = WIN;
		gameStatus(&gStatus);
	}
	else{
		if(Estat_row[row].Epr == 1){
			lastCheck = 1;			//Does forward checking only
		}
		else if(Estat_row[row].Epr == 0){
			AliveRows[row] = 0;
		}

		if(Estat_row[row].Epr){
			unsigned char column=0;
			unsigned char firstCheck = 0;
			for(column=0;column<MAX_ENEMY_PR;column++){
				if ((firstCheck == 0)&&(Enemy[row][column].life)){									//counts forward, check = 0 >> keeps checking
					Estat_row[row].Fep = column;
					firstCheck = 1;
					if(Estat_row[row].Epr == 1){
						Estat_row[row].Lep=Estat_row[row].Fep;
					}
				}
				
				if((lastCheck == 0)&&(Enemy[row][REAL_MAX_EPR-column].life)){		//counts backwards
					Estat_row[row].Lep = REAL_MAX_EPR-column;
					lastCheck = 1;
				}	
				
				if((firstCheck)&&(lastCheck)){
					break;
				}
			}
		}
	}

	{unsigned int *alr_counter;
		{unsigned char i;
			for(i=0;i<=MAXROWS-1;i++){
				if(AliveRows[i]){
					alr_counter++;
				}
			}
		}
		return alr_counter;
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
static unsigned Verify_lastLine(unsigned lastLine){
	while(Estat_row[lastLine].Epr == 0){
		lastLine--;
	}
	return lastLine;
}
#endif
//********FirstEPC*****************
// Keep track of the first enemy per column
// changes: Estat_column[column].(Fep|Epc),AlColsMat[aliveCol], LiveCols
// Callers: EnemyLaserInit, EnemyscanX
// inputs: mode = RETURNVAL|UPDATE|RETURNARR|RESET
// outputs: LiveCols
// assumes: na
#if DRAW_ENEMIES
unsigned int * FirstEPC(unsigned char mode){
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
		signed char row = Estat_column[column].Fep;		//start from last known position
		unsigned char quit = 0;
		if(Estat_column[column].Epc == 0){
			continue;
		}
	
		AlColsMat[aliveCol] = column;
		aliveCol++;
		
		//finds the first enemy on a column
		while(quit==0 && row>=0){
			if(Enemy[row][column].life){
				Estat_column[column].Fep = row;
				quit= 1;
			}
			else{
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
//--------------------------------------------------------------Miscelaneus----------------------------------------------------------
//********enemyBonusCreate*****************
// It creates the Enemy bonus ship, and times the reApereance
// changes: na
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMYBONUS
	void enemyBonusCreate(void){
		static unsigned char localCounter = 0;
		
		if((EnemyBonus.life == 0) && (localCounter >= BONUSTIMING)){
			BonusEnemyInit();
			localCounter = 0;
		}
		
		if(EnemyBonus.life == 0){
			localCounter++;
		}
	}
#endif
//--------------------------------------------------------------Utilities----------------------------------------------------------
//********absValue*****************
// Returns positive value
// changes: value
// inputs: value
// outputs: value
// assumes: na
signed char absValue(signed char value){
	if(value<0){
		value = -value;
	}
	return value;
}
//********gameStatus*****************
// Systick sets the game status on the Engine
// changes: na
// Callers: systick, FirstLast, EnemyLaserCollisions
// inputs: none
// outputs: none
// assumes: na
void gameStatus(volatile unsigned int *p){
	gStatus = *p;
}
//********whatStatus*****************
// Returns the current game status
// changes: na
// Callers: systick
// inputs: none
// outputs: none
// assumes: na
volatile unsigned int * whatStatus(void){
	volatile static  unsigned int *p = NULL;
	return p = &gStatus;
}
//---------------------------------------------------------------TODOS--------------------------------------------------
/*
changes:
	*add sound:
		-lab13
Improve firing: adding firing secuences

*/



/*
-------------------------------------------------------------Var Map Globals---------------------------------------------
I created this section to identify the needs to re-scoping.
>: Static value modified to default(Special care or consideration)

Reason to remain: There are several function that need this variable (a entire branch Collision), then it updates at the end of the branch 
(Verify_lastLine), and it is also used on a separate branch that needs the value already updated.
Var: lastLine
updated@:Verify_lastLine
Functions:
		Verify_lastLine
		EnemyscanX
		EnemyscanY
		defaultValues
		Enemy_Move
		EnemyShiftTrack
		PlayerLaserCollisions
		
		notes:
		End game@: FirstLast, EnemyLaserCollisions@MasterDraw
		
		http://www.keil.com/support/man/docs/armcc/armcc_chr1359124211145.htm

*/
