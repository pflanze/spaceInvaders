//GameEngine.h




//----------------------------------------------------------Definition------------------------------------------------
//debugging code
#define DRAW_ENEMIES	1
#define DRAW_ENEMYBONUS 1

//used for Convert2Distance calibration
#define Avalue 16
#define Bvalue 0

//game max
#define MAXLASERS 5
#define MAX_ENEMY_PR 4
#define MAXROWS	1

//collision offset
#define YOFFSET 6
#define XOFFSET 2

//screen specs
#define LOWERLIMIT 47
#define RIGHTLIMIT 68
#define LEFTLIMIT 0
#define TOPLIMIT 6

//Ship collision line
#define SHIPCOLISIONLINE LOWERLIMIT-PLAYERH+2

//ship midPoints
#define SHIP_MIDP ((PLAYERW>>1)-1)

//BonusShip
#define BONUS_C_LINE TOPLIMIT+ENEMYBONUSH
#define BONUSTIMING 1
#define OFFSETEXPLOSIONX 2
#define OFFSETEXPLOSIONY 4

//BonusShip midPoint xCoordinate
#define ENEMY_Bon_midX (ENEMYBONUSW>>1)

//enemy midPoint xCoordinate
#define E_LASER_OFFX ((ENEMY30W>>1)-1)
#define E_LASER_OFFY 5

//Game over status
#define INGAME	0
#define LOOSE		1
#define WIN			2
#define STANDBY	3

//Objecdt IDs
#define ID_SHIP			0
#define ID_ENEMY		1
#define ID_BONUS		2
#define ID_E_LASER	3
#define ID_S_LASER	4



//----------------------------------------------------------Structs------------------------------------------------



struct State {
	unsigned char x;      // x coordinate
	unsigned char y;      // y coordinate
	const unsigned char *image[2]; // two pointers to images
	unsigned char life :1;            // 0=dead, 1=alive
	unsigned char JK :1;	//status for image replacement, represents "Just Killed"				, needs updating
	unsigned char id;
};         

typedef struct State STyp;

//----------------------------------------------------------Funtion definition------------------------------------------------
void Player_Move(void);
void LaserInit_ship(void);
void ShipInit(void);
void MoveObjects(void);
void Draw(void);
unsigned long Convert2Distance(unsigned long sample);
unsigned long ADC0_In(void);


#if DRAW_ENEMIES
	void EnemyInit(void);
	void Enemy_Move(unsigned char LeftShiftColumn, unsigned char RightShiftColumn);
	void EnemyShiftTrack(void);
	void EnemyLaserCollisions(void);
	void LaserCollision(void);
	void LaserEnemy_Move(void);
	void PlayerLaserCollisions(void);
#endif

#if DRAW_ENEMYBONUS
	void BonusEnemyInit(void);
	void BonusEnemy_Move(void);
	void enemyBonusCreate(void);
#endif

void CheckingCollisions(void);
void BonusLaserCollision(void);
void LaserShip_Move(void);
void EnemyscanY(unsigned char laserNum);
unsigned char EnemyscanX(unsigned char row, unsigned char laserNum);
void FirstLast(unsigned char row, unsigned char column);
void EnemyDraw(void);
void LaserShipDraw(void);
void defaultValues(void);
unsigned char queryLiveRows(void);
unsigned char LastL(void);
void FirstEPC(void);
void EnemyLaserInit(void);
void LaserEnemyDraw(void);
signed char absValue(signed char value);

void MasterDraw(struct State *st_ptr);

