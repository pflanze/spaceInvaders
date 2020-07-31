//GameEngine.h
#ifndef _GAMEENGINE_H
#define _GAMEENGINE_H
#include "debug.h"

//----------------------------------------------------------Definition------------------------------------------------
//game max
#define MAXLASERS 5
#define MAX_ENEMY_PR 4
#define REAL_MAX_EPR (MAX_ENEMY_PR-1)
#define ALLOC_MAXROWS	2

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

//Game over status
#define INGAME		0
#define LOOSE			1
#define WIN				2
#define STANDBY		3
#define RESET			4


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
