//GameEngine.h
#ifndef _GAMEENGINE_H
#define _GAMEENGINE_H
#include "debug.h"
#include "utils.h" /* bool */
#include "object.h"


//Game over status
#define INGAME		0
#define LOOSE		1
#define WIN			2
#define STANDBY		3
//0: In game
//1: Game Over (you loose)
//2: Just Won
//3: Game on standBy

//firing delay
#define FIREDEL 	7


// (private, only here since used in the struct definitions)
//game max
#define MAXLASERS 	 	5
#define MAX_ENEMY_PR  	4
#define ALLOC_MAXROWS	2



struct Actor {
	const struct ObjectInterface* vtable;
	unsigned char x;               // x coordinate
	unsigned char y;               // y coordinate
	const unsigned char *image[2]; // two pointers to images
	bool alive;
	bool jk;                       // status for image replacement,
	                               // represents "Just Killed", needs updating
	unsigned char id;
};

#ifdef DEBUG
const struct ObjectInterface Actor_ObjectInterface;
#endif

static inline
void Actor_init(struct Actor* this,
				struct Actor values) {
	*this = values;
#ifdef DEBUG
	this->vtable = &Actor_ObjectInterface;
#endif
}


#ifdef DEBUG
const char* Actor_id_string(struct Actor *this);
#endif



struct GameStatColumn {
	const struct ObjectInterface* vtable;
	unsigned char fep;		//"First enemy position"
	unsigned char epc;		//"Enemies per column"
};

#ifdef DEBUG
const struct ObjectInterface GameStatColumn_ObjectInterface;
#endif

static inline
void GameStatColumn_init(struct GameStatColumn* this,
						 struct GameStatColumn values) {
	*this = values;
#ifdef DEBUG
	this->vtable = &GameStatColumn_ObjectInterface;
#endif
}


//game stats per row
struct GameStatRow {
	const struct ObjectInterface* vtable;
	unsigned char fep;		//"First enemy position"
	unsigned char lep;		//"Last enemy position"
	unsigned char epr;		//"Enemies per row"
};

#ifdef DEBUG
const struct ObjectInterface GameStatRow_ObjectInterface;
#endif

static inline
void GameStatRow_init(struct GameStatRow* this,
					  struct GameStatRow values) {
	*this = values;
#ifdef DEBUG
	this->vtable = &GameStatRow_ObjectInterface;
#endif
}


struct GameEngine {
	const struct ObjectInterface* vtable;
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
	struct GameStatColumn gameStatColumn[MAX_ENEMY_PR];
	struct GameStatRow gameStatRow[ALLOC_MAXROWS];
	struct Actor enemy[ALLOC_MAXROWS][MAX_ENEMY_PR];
	struct Actor laser_enemy[MAXLASERS];
#endif

	struct Actor ship;
	struct Actor laser_ship[MAXLASERS];

#if DRAW_BONUSENEMY
	struct Actor bonusEnemy;
#endif

	// GameEngine_enemy_move:
	bool right; //moves the enemies, 0: moves left
	bool down; //moves the enemies, 1: moves down

	// GameEngine_enemyDraw:
	unsigned char frameIndex;  // 0,1,0,1,...

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
	bool rowAlive[ALLOC_MAXROWS];

	// GameEngine_firstEPC:
	unsigned int liveCols;
	unsigned int alColsMat[MAX_ENEMY_PR];

#if DRAW_BONUSENEMY
	// GameEngine_bonusEnemyCreate:
	unsigned char localCounter;
#endif
};


void GameEngine_player_move(struct GameEngine *this);
void GameEngine_shipLasers_init(struct GameEngine *this);
void GameEngine_shipLasersCreation(struct GameEngine *this, bool init);
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
						   unsigned int leftShiftColumn,
						   unsigned int lightShiftColumn);
void GameEngine_defaultValues(struct GameEngine *this);
#endif

#if DRAW_BONUSENEMY
void GameEngine_bonusEnemy_init(struct GameEngine *this);
void GameEngine_bonusEnemy_move_reset(struct GameEngine *this);
void GameEngine_bonusEnemy_move(struct GameEngine *this);
void GameEngine_bonusEnemyCreate(struct GameEngine *this);
#endif

void GameEngine_bonusLaserCollision(struct GameEngine *this);
void GameEngine_laserShip_move(struct GameEngine *this);
void GameEngine_enemyDraw(struct GameEngine *this);
void GameEngine_laserShipDraw(struct GameEngine *this);

void GameEngine_enemyLasers_init(struct GameEngine *this);
void GameEngine_enemyLasersCreation(struct GameEngine *this, bool init);
void GameEngine_laserEnemyDraw(struct GameEngine *this);
void GameEngine_masterDraw(struct GameEngine *this,
						   struct Actor *s,
						   unsigned int frameIndex);

void GameEngine_enemyscanY(struct GameEngine *this,
						   unsigned int laserNum);

signed int absValue(signed int value);
unsigned long Convert2Distance(unsigned long sample);

void GameEngine_moveObjects(struct GameEngine *this);

void GameEngine_collisions(struct GameEngine *this);
void GameEngine_reset(struct GameEngine *this);

#if DRAW_ENEMIES
void GameEngine_enemyTracking_reset(struct GameEngine *this);
void GameEngine_enemyShiftTrack(struct GameEngine *this,
								unsigned int localAliveRows);
void GameEngine_firstLast_reset(struct GameEngine *this);
unsigned int GameEngine_firstLast(struct GameEngine *this,
								  unsigned int row,
								  unsigned int column);
// first enemy per column
void GameEngine_firstEPC_reset(struct GameEngine *this);
void GameEngine_firstEPC(struct GameEngine *this);
#endif
void GameEngine_enemyscanX(struct GameEngine *this,
						   unsigned int row,
						   unsigned int laserNum);
void GameEngine_setStatus(struct GameEngine *this, unsigned int v);
unsigned int GameEngine_getStatus(struct GameEngine *this);
void GameEngine_laserInit_ship2(struct GameEngine *this);
void GameEngine_init(struct GameEngine *this,
					 unsigned int max_number_of_enemy_rows);

#endif /* _GAMEENGINE_H */
