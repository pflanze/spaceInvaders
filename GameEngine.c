//GameEngine.c

#include "GameEngine.h"
#include "sprites.h"
#include "Nokia5110.h"
#include "..//tm4c123gh6pm.h"
#include "Random.h"

extern unsigned char gameOverFlag;		//0: Keep playing

//local variables
unsigned char FrameCount=0;
unsigned char right = 1;			//moves the enemies, 0: moves left
unsigned char down = 0;			//moves the enemies, 1: moves down

//----------------------------------------------------------Structs------------------------------------------------
//game stats per column
struct GameStatColumn {
	unsigned char Fep;		//"First enemy position"
	unsigned char Epc;		//"Enemies per column"
};

typedef struct GameStatColumn Ctype; 
Ctype Estat_column[MAX_ENEMY_PR];

//game stats per row
struct GameStatRow {
	unsigned char Fep;		//"First enemy position"
	unsigned char Lep;		//"Last enemy position"
	unsigned char Epr;		//"Enemies per row"
};

typedef struct GameStatRow Etype; 
Etype Estat_row[MAXROWS];

STyp Ship;
STyp Laser_ship[MAXLASERS];

#if DRAW_ENEMIES
	//use to keep the statistics from the game
	static unsigned char enemyTracking[] = {0,3};					//keeps track of the first and last enemy across diferrent rows
	static unsigned lastLine = MAXROWS-1;
	static unsigned char LiveRows[MAXROWS];
	static unsigned char enemyCount = MAXROWS*MAX_ENEMY_PR;
	
	STyp Enemy [MAXROWS][MAX_ENEMY_PR];
	STyp Laser_enemy[MAXLASERS];
#endif

#if DRAW_ENEMYBONUS
	STyp EnemyBonus;
#endif	
	
//-----------------------------------------------------------INIT-----------------------------------------------------------------------
//********EnemyInit*****************
//Initialize enemies
//Enemies are counted normaly, top left 00
// changes: Enemy[row][column].*, Estat_row[row].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
	void EnemyInit(void){
		unsigned int column=0;
		unsigned char row=0;
		
		while(row<MAXROWS){
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
			Estat_row[row].Epr = MAX_ENEMY_PR;	//keeps track od the amount of enemies per row
			Estat_row[row].Fep = 0;							//first end position
			Estat_row[row].Lep = 3;							//last end position
			row++;
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
	Ship.x = 48;
	Ship.y = 46;
	Ship.image[0] = PlayerShip0;
	Ship.life = 1;				// 0=dead, 1=alive
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
			break;			//terminate loop when a slot is found
		}
	}	
}
//********EnemyLaserInit*****************
//Initializes the lasers fired by the enemy ship. It selects an enemy randomly to shoot.
//Used by: Draw(LaserEnemyDraw)
// changes: Laser_enemy[index].*
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
	void EnemyLaserInit(void){
	unsigned char  columnNew = (Random32()>>24)%4;		//generates number [0-3]
		
		if(Estat_column[columnNew].Epc){
			unsigned char row = Estat_column[columnNew].Fep;
			unsigned char i;
		
			for(i=0;i<MAXLASERS;i++){
				if(Laser_enemy[i].life == 0){
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
// Multiline description
// changes: variablesChanged
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
// changes: enemyTracking[], Laser_ship[i].life, Laser_enemy[i].life, LiveRows[i], lastLine, Estat_column[i].(Epc|Fep)
// inputs: none
// outputs: none
// assumes: na
void defaultValues(void){
	unsigned char i;

	//tracking defaults
	#if DRAW_ENEMIES
		enemyTracking[0] = 0;
		enemyTracking[1] = 3;
	
		enemyCount = MAXROWS*MAX_ENEMY_PR;
		lastLine = MAXROWS-1;
	
		//sets defaults for FirstEPC
		for(i=0;i<MAX_ENEMY_PR;i++){
			Estat_column[i].Epc = MAXROWS;
			Estat_column[i].Fep = lastLine;
		}	
		
		for(i=0;i<MAXLASERS;i++){											
			Laser_enemy[i].life = 0;
		}
		
		//liverows[] defaults
		for(i=0;i<MAXROWS;i++){
			LiveRows[i] = 1;
		}
	#endif
		
	#if DRAW_ENEMYBONUS
		EnemyBonus.life = 0;
	#endif
		
	//laser defaults
	for(i=0;i<MAXLASERS;i++){											
		Laser_ship[i].life = 0;
	}
	
	//Ship defaults
	Ship.image[0] = PlayerShip0;
	Ship.life = 1;				// 0=dead, 1=alive
}
//--------------------------------------------------------------MOVE OBJECTS-------------------------------------------------------------------------
//********MoveObjects*****************
//Updates the position values of the asociated objects
// inputs: none
// outputs: none
// assumes: na
void MoveObjects(void){
	if((gameOverFlag == INGAME)||(gameOverFlag == STANDBY)){
		Player_Move();						//calls ADC0_in, Convert2Distance
		
		if(gameOverFlag == INGAME){
			#if DRAW_ENEMIES
				Enemy_Move(enemyTracking[0], enemyTracking[1]);					//updates enemy coordinate
				LaserEnemy_Move();
			#endif
			
			#if DRAW_ENEMYBONUS
				BonusEnemy_Move();
			#endif
			LaserShip_Move();
		}
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
// changes: Enemy[row][Column].[x|y], gameOverFlag
// inputs: LeftShiftColumn, RightShiftColumn
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void Enemy_Move(unsigned char LeftShiftColumn, unsigned char RightShiftColumn){ 
	signed char column = 0;
	unsigned char row = 0;
	
	while(row<MAXROWS){
		if(Enemy[lastLine][0].y < 40){			//Do it while not raching the earth, At 40 the ships have reach the earth!
			
			//sets the switches to move down/left/right
			if(Enemy[row][RightShiftColumn].x >= RIGHTLIMIT){
				right = 0;	//moves left
				down = 1;
			}
			else if(Enemy[row][LeftShiftColumn].x <= LEFTLIMIT){
				right = 1;
				down = 1;
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
				down = 0;
			}
		}
		else{										//Enemies have reached the earth
			gameOverFlag = LOOSE;
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
void BonusEnemy_Move(void){
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
		
		//drawing enemies
		#if DRAW_ENEMIES
			if(gameOverFlag == INGAME){
				EnemyDraw();											//Uses MasterDraw
				FrameCount = (FrameCount+1)&0x01; // 0,1,0,1,...
			}
			else if(gameOverFlag == STANDBY){
				EnemyDraw();			//Uses MasterDraw
			}
			LaserEnemyDraw();		//Uses MasterDraw
		#endif
		
		//drawing battleship
		MasterDraw(&Ship);
			
		//drawing laser
		LaserShipDraw();		//uses MasterDraw
		
		#if DRAW_ENEMYBONUS		
			MasterDraw(&EnemyBonus);
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
	unsigned char column=0;
	signed char row=0;

	while(row<MAXROWS){
		for(column=0;column<4;column++){	
			MasterDraw(&Enemy[row][column]);
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
void MasterDraw(struct State *st_ptr){
	static unsigned char frame = 0;
	signed char offsetX = 0;
	signed char offsetY = 0;
	
	if(st_ptr->JK){
		//used to change explosions offset values
		if(st_ptr->id == ID_BONUS){	//BONUS
			offsetX = OFFSETEXPLOSIONX;
			offsetY = OFFSETEXPLOSIONY;
		}
		else if(st_ptr->id == ID_E_LASER){	//BONUS
			offsetX = -5;
		}
		
		switch (frame){
		case 0:
			st_ptr->image[0] = SmallExplosion0;
			break;
		case 1:
			st_ptr->image[1] = SmallExplosion1;
			break;
		case 2:
			st_ptr->JK = 0;
			frame = 0;
			if(st_ptr == &Ship){
				gameOverFlag = LOOSE;
			}
			break;
		}
	}
	if(st_ptr->life){
		//if I tried to have "frame = FrameCount" insde the if... and have a singgle line for the nokia... everythink start blinking like crazy
		//However I did not try to access FrameCount by reference
		if(st_ptr->id == ID_ENEMY){		//only enemies need change between frames
			Nokia5110_PrintBMP(st_ptr->x, st_ptr->y, st_ptr->image[FrameCount], 0); //frame is always 0, except for enemies
		}
		else{
			Nokia5110_PrintBMP(st_ptr->x, st_ptr->y, st_ptr->image[0], 0); //frame is always 0, except for enemies
		}	
	}
	else if(st_ptr->JK){
		Nokia5110_PrintBMP(st_ptr->x + offsetX, st_ptr->y + offsetY, st_ptr->image[frame], 0);
		frame++;
	}
}
//********LaserShipDraw*****************
//Updates position of alive lasers
// inputs: none
// outputs: none
// assumes: na
void LaserShipDraw(void){
	unsigned char laserNum = 0;
	
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
		if(Laser_ship[laserNum].life){
			MasterDraw(&Laser_ship[laserNum]);
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
	unsigned char laserNum = 0;
	for(laserNum=0;laserNum<MAXLASERS;laserNum++){
				MasterDraw(&Laser_enemy[laserNum]);
	}
}
#endif
//********EnemyShiftTrack*****************
//Used to calculate the position the enemy next position
// changes: enemyTracking[], 
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void EnemyShiftTrack(void){
	signed char row = 0;
	static unsigned char highest = 3;		//represents the higest column number with a enemy alive (general)
	static unsigned char lowest = 0;		//represents the lowest column number with a enemy alive (general)
	
	switch(queryLiveRows()){
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
	
		default:
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
//********CheckingCollisions*****************
//Check colision detection: Enemy lasers, ship lasers, laser on laser
// inputs: none
// outputs: none
// assumes: na
void CheckingCollisions(void){
	if(gameOverFlag == INGAME){
		#if DRAW_ENEMYBONUS
			BonusLaserCollision();
		#endif
		
		#if DRAW_ENEMIES
			PlayerLaserCollisions();
			EnemyLaserCollisions();
			LaserCollision();
		#endif
	}
}
//********PlayerLaserCollisions*****************
//Checks if the laser have a hit to an enemy (for every laser)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void PlayerLaserCollisions(void){
	unsigned char laserNum = 0; 
	unsigned char row = 0; 

	if(gameOverFlag == INGAME){
		//each laser is checked for a collition
		for(laserNum=0;laserNum<MAXLASERS;laserNum++){
			if((Laser_ship[laserNum].life)&&(Enemy[lastLine][0].y + YOFFSET >= Laser_ship[laserNum].y)){//found a line with enemies>>start calculating
				if(Estat_row[row].Epr == 0){
					break;
				}
				//calculate enemy zone(grouping)
				EnemyscanY(laserNum);	
			}	
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
	
	row = lastLine;//temporal reset
		
	while((found == 0)&&(exit == 0)){
		if(Laser_ship[laserNum].y > ELL[row]){
			exit = 1;
		}
		else if(Laser_ship[laserNum].y < ELH[row]){
		 row--;
		}
		else{
			if(EnemyscanX(row, laserNum)){
				found = 1;
			}
			else{							//temporal testing
				exit = 1;
			}
		}
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
unsigned char EnemyscanX(unsigned char row, unsigned char laserNum){
	unsigned char column = 0; 
	unsigned char enemyDestroyed = 0;
	
	while((enemyDestroyed == 0)&&(column <= Estat_row[row].Lep)){
		if(Enemy[row][column].life){
			//checking x coordinate of each active laser against each enemy
			signed char enemyInRange = (Enemy[row][column].x + E_LASER_OFFX - Laser_ship[laserNum].x);
			enemyInRange = absValue(enemyInRange);
			if (enemyInRange <= E_LASER_OFFX){	
				Laser_ship[laserNum].life = 0;
				Enemy[row][column].life = 0;
				Enemy[row][column].JK = 1;
				FirstLast(row, column);
				LastL();								//updates last line value
				#if DRAW_ENEMIES
					EnemyShiftTrack();
				#endif
				FirstEPC();
				enemyDestroyed = 1;
			}
		}
		column++;
	}	
	return enemyDestroyed;
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
	unsigned char i = 0;
	
	if(Ship.life){
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
					return;
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
	unsigned char lasernumShip = 0;
	unsigned char lasernumEnemy = 0;
	
	//checks collision course
	for(lasernumEnemy=0;lasernumEnemy<MAXLASERS;lasernumEnemy++){
		if(Laser_enemy[lasernumEnemy].life){																										//avoids unnecessary comparison
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
//The last and first elements representing the enemies alive on the respaective positions
//The function will return 0x0000 when there is not more enemies alive (row)
//Meaning that it aslo keeps track of the enemies alive in rows and columns (counter)
//The function uses left and right shifting for the return value
//this function should keep track of the number of enemies on each row
// changes: Estat_row[row].*, Estat_column[column].Epc, enemyCount, gameOverFlag, LiveRows[row]
// inputs: row, column
// outputs: none
// assumes: na
#if DRAW_ENEMIES
void FirstLast(unsigned char row, unsigned char column){
	
	//used to verify the alive position enemies only once
	unsigned char firstCheck = 0;
	unsigned char lastCheck = 0;
	
	Estat_row[row].Epr--;
	Estat_column[column].Epc--;
	enemyCount--;
	
	if(enemyCount == 0){
		LiveRows[row] = 0;					//needed only to update stats before quiting, good for debugging
		gameOverFlag = WIN;
	}
	else{
		if(Estat_row[row].Epr == 1){
			lastCheck = 1;			//Does forward checking only
		}
		else if(Estat_row[row].Epr == 0){
			LiveRows[row] = 0;
		}

		if(Estat_row[row].Epr){
			unsigned char column=0;
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
}
#endif
//********queryLiveRows*****************
// Returns the number of rows alive
// inputs: none
// outputs: lr_counter
// assumes: na
#if DRAW_ENEMIES
	unsigned char queryLiveRows(void){
		signed char i;
		unsigned char lr_counter = 0;		//number of rows alive
		
		for(i=0;i<=MAXROWS-1;i++){
			if(LiveRows[i]){
				lr_counter++;
			}
		}
		return lr_counter;
	}
#endif
//********LastL*****************
//Keeps track of the last enemy line
// changes: lastLine
// inputs: none
// outputs: lastLine
// assumes: na
#if DRAW_ENEMIES
	unsigned char LastL(void){

	while(Estat_row[lastLine].Epr == 0){
		lastLine--;
	}
		return lastLine;
	}
#endif
//********FirstEPC*****************
// Keep track of the first enemy per column
// changes: Estat_column[column].(Fep|Epc)
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMIES
	void FirstEPC(void){
		unsigned char column = 0;
		
		//we are reading left>right, dowun>up
		for(column=0;column<MAX_ENEMY_PR;column++){
			signed char row = Estat_column[column].Fep;		//start from last known position
			unsigned char quit = 0;
			if(Estat_column[column].Epc == 0){
				continue;
			}
			
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
	}
#endif
//--------------------------------------------------------------Miscelaneus----------------------------------------------------------
//********functionName*****************
// Multiline description
// changes: variablesChanged
// inputs: none
// outputs: none
// assumes: na
#if DRAW_ENEMYBONUS
	void enemyBonusCreate(void){
		static unsigned localCounter = 0;
		
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
// changes: no changes, only returns
// inputs: value
// outputs: value
// assumes: na
signed char absValue(signed char value){
	if(value<0){
		value = -value;
	}
	return value;
}
//---------------------------------------------------------------TODOS--------------------------------------------------
/*
changes:
	*Enemy random.. decrease range as they get killed
	*add sound:
		-lab13
	*Try to make it scalable
	*defaults and init may be duplicated
	*Pass by reference:
		- Before changing anything: check addresses on watch window
		-EnemyscanX, FirstLast, EnemyscanY, Enemy_Move, Convert2Distance, absValue

note!!!
	*I created a temporal function to test passing by reference 2D-array of structs

*/
