#ifndef BULLET_H
#define BULLET_H

typedef struct tbullet {
	s16 x;
	s16 y;
	s16 vx;
	s16 vy;
	u8	type;
	u8	state;
	u8	behavior;
	u8	angle;
	
	u8	target;			// uses collision system for targets
	u8	damage;
	u8	tile;
	u8	blink;
	
	u8	src;
	u8	r1;
	u8	r2;
	u8	r3;
} bullet;

#define BEHAVIOR_STATIC 0
#define BEHAVIOR_SMART 1
#define BEHAVIOR_SPEC1 2
#define BEHAVIOR_EXPLOSION 3
#define MAX_BULLETS 70

void bulletsInit( void );
void bulletsClear( void );
void bulletFireP(  int x, int y, int vx, int vy, int type, int behavior, int target, int damage, int tile, int player );
void bulletFire( int x, int y, int vx, int vy, int type, int behavior, int target, int damage, int tile );
void bulletsRefresh( void );
void bulletsDraw( void );

extern bullet bullets[];
extern int next_bullet;

#define bullet_tile_entry	130
#define bullet_tile_small	0
#define bullet_tile_large	1
#define bullet_tile_missile	2
#define bullet_tile_p1		3
#define bullet_tile_p2		4
#define bullet_tile_p3		5
#define bullet_tile_bomb	6

#endif
