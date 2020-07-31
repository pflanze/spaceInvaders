//GameEngine.h
#ifndef _GAMEENGINE_H
#define _GAMEENGINE_H
#include "debug.h"


//----------------------------------Definitions---------------------------------

//Game over status
#define INGAME		0
#define LOOSE			1
#define WIN				2
#define STANDBY		3
#define RESET			4

//firing delay
#define FIREDEL 		7


//----------------------------------------------------------Structs------------------------------------------------

struct State {
	unsigned char x;      // x coordinate
	unsigned char y;      // y coordinate
	const unsigned char *image[2]; // two pointers to images
	unsigned char life;            // 0=dead, 1=alive
	unsigned char JK;	//status for image replacement, represents "Just Killed"				, needs updating
	unsigned char id;
};         

//----------------------------------------------------------Function definition------------------------------------------------
void Player_Move(void);
void LaserInit_ship(void);
void ShipInit(void);
void Draw(void);
unsigned long ADC0_In(void);

#ifdef TEST_WITHOUT_IO
extern unsigned long ADC0_SSFIFO3_R;
#endif


#if DRAW_ENEMIES
	void EnemyInit(void);
	void LaserCollision(void);
	void LaserEnemy_Move(void);
	void PlayerLaserCollisions(void);
	void Enemy_Move(unsigned int LeftShiftColumn, unsigned int RightShiftColumn);
	void defaultValues(void);
#endif

#if DRAW_ENEMYBONUS
//	void BonusEnemyInit(void);
	void BonusEnemy_Move(unsigned int mode);
	void enemyBonusCreate(void);
#endif

void BonusLaserCollision(void);
void LaserShip_Move(void);
void EnemyDraw(void);
void LaserShipDraw(void);

void EnemyLaserInit(void);
void LaserEnemyDraw(void);
void MasterDraw(struct State *st_ptr, unsigned int FrameCount);

void EnemyscanY(unsigned int laserNum);

signed int absValue(signed int value);
unsigned long Convert2Distance(unsigned long sample);

void MoveObjects(void);

void Collisions(void);
void reset(void);

unsigned int * EnemyShiftTrack(unsigned int localAliveRows, unsigned int mode);
unsigned int FirstLast(unsigned int row, unsigned int column, unsigned int mode);
unsigned int * FirstEPC(unsigned int mode);
void EnemyscanX(unsigned int row, unsigned int laserNum);
void setStatus(unsigned int v);
unsigned int getStatus(void);
void LaserInit_ship2(void);
void GameEngine_init(unsigned int max_number_of_enemy_rows);

#endif /* _GAMEENGINE_H */
