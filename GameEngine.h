//GameEngine.h
#ifndef _GAMEENGINE_H
#define _GAMEENGINE_H
#include "debug.h"
#include "utils.h" /* bool */


//----------------------------------Definitions---------------------------------

//Game over status
#define INGAME		0
#define LOOSE			1
#define WIN				2
#define STANDBY		3
#define RESET			4
//0: In game
//1: Game Over (you loose)
//2: Just Won
//3: Game on standBy

//firing delay
#define FIREDEL 		7


// (private, only here since used in the struct definitions)
//game max
#define MAXLASERS 5
#define MAX_ENEMY_PR 4
#define ALLOC_MAXROWS	2


//-----------------------------------Structs------------------------------------

struct Actor {
	unsigned char x;               // x coordinate
	unsigned char y;               // y coordinate
	const unsigned char *image[2]; // two pointers to images
	bool alive;
	unsigned char JK;              // status for image replacement,
	                               // represents "Just Killed", needs updating
	unsigned char id;
};


struct GameStatColumn {
	unsigned char Fep;		//"First enemy position"
	unsigned char Epc;		//"Enemies per column"
};

//game stats per row
struct GameStatRow {
	unsigned char Fep;		//"First enemy position"
	unsigned char Lep;		//"Last enemy position"
	unsigned char Epr;		//"Enemies per row"
};

struct GameEngine {
	unsigned int gStatus;
	unsigned int maxrows;

#if DRAW_ENEMIES
	//use to keep the statistics from the game
	unsigned lastLine;
	// GameEngine_firstLast:
	unsigned char enemyCount;
#endif

	//game stats per column
#if DRAW_ENEMIES
	struct GameStatColumn Estat_column[MAX_ENEMY_PR];
	struct GameStatRow Estat_row[ALLOC_MAXROWS];
	struct Actor Enemy[ALLOC_MAXROWS][MAX_ENEMY_PR];
	struct Actor Laser_enemy[MAXLASERS];
#endif

	struct Actor Ship;
	struct Actor Laser_ship[MAXLASERS];

#if DRAW_ENEMYBONUS
	struct Actor  EnemyBonus;
#endif

	// GameEngine_enemy_move:
	bool right; //moves the enemies, 0: moves left
	bool down; //moves the enemies, 1: moves down

	// GameEngine_enemyDraw:
	unsigned char FrameCount;  // 0,1,0,1,...

	// GameEngine_masterDraw:
	unsigned char frame;

	// GameEngine_enemyShiftTrack:
	unsigned int enemyTracking[2];
	// ^ keeps track of the first and last enemy across diferrent rows
	unsigned char lowest;
	// ^ represents the lowest column number with a enemy alive (general)
	unsigned char highest;
	// ^ represents the higest column number with a enemy alive (general)

	// GameEngine_firstLast:
	bool AliveRows[ALLOC_MAXROWS];

	// GameEngine_firstEPC:
	unsigned int LiveCols;
	unsigned int AlColsMat[MAX_ENEMY_PR];

#if DRAW_ENEMYBONUS
	// GameEngine_enemyBonusCreate:
	unsigned char localCounter;
#endif
};


//----------------------------------Function definition-------------------------
void GameEngine_player_move(struct GameEngine *this);
void GameEngine_laserInit_ship(struct GameEngine *this);
void GameEngine_shipInit(struct GameEngine *this);
void GameEngine_draw(struct GameEngine *this);
unsigned long ADC0_In(void);

#ifdef TEST_WITHOUT_IO
extern unsigned long ADC0_SSFIFO3_R;
#endif


#if DRAW_ENEMIES
void GameEngine_enemyInit(struct GameEngine *this);
void GameEngine_laserCollision(struct GameEngine *this);
void GameEngine_laserEnemy_move(struct GameEngine *this);
void GameEngine_playerLaserCollisions(struct GameEngine *this);
void GameEngine_enemy_move(struct GameEngine *this,
			   unsigned int LeftShiftColumn,
			   unsigned int RightShiftColumn);
void GameEngine_defaultValues(struct GameEngine *this);
#endif

#if DRAW_ENEMYBONUS
void GameEngine_bonusEnemyInit(struct GameEngine *this);
void GameEngine_bonusEnemy_Move(struct GameEngine *this, unsigned int mode);
void GameEngine_enemyBonusCreate(struct GameEngine *this);
#endif

void GameEngine_bonusLaserCollision(struct GameEngine *this);
void GameEngine_laserShip_move(struct GameEngine *this);
void GameEngine_enemyDraw(struct GameEngine *this);
void GameEngine_laserShipDraw(struct GameEngine *this);

void GameEngine_enemyLaserInit(struct GameEngine *this);
void GameEngine_laserEnemyDraw(struct GameEngine *this);
void GameEngine_masterDraw(struct GameEngine *this,
			   struct Actor *s,
			   unsigned int FrameCount);

void GameEngine_enemyscanY(struct GameEngine *this,
			   unsigned int laserNum);

signed int absValue(signed int value);
unsigned long Convert2Distance(unsigned long sample);

void GameEngine_moveObjects(struct GameEngine *this);

void GameEngine_collisions(struct GameEngine *this);
void GameEngine_reset(struct GameEngine *this);

unsigned int * GameEngine_enemyShiftTrack(struct GameEngine *this,
					  unsigned int localAliveRows,
					  unsigned int mode);
unsigned int GameEngine_firstLast(struct GameEngine *this,
				  unsigned int row,
				  unsigned int column,
				  unsigned int mode);
unsigned int * GameEngine_firstEPC(struct GameEngine *this, unsigned int mode);
void GameEngine_enemyscanX(struct GameEngine *this,
			   unsigned int row,
			   unsigned int laserNum);
void GameEngine_setStatus(struct GameEngine *this, unsigned int v);
unsigned int GameEngine_getStatus(struct GameEngine *this);
void GameEngine_laserInit_ship2(struct GameEngine *this);
void GameEngine_init(struct GameEngine *this,
		     unsigned int max_number_of_enemy_rows);

#endif /* _GAMEENGINE_H */
