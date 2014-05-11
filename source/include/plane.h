#ifndef PLANE_H
#define PLANE_H

#include "comms.h"

typedef struct tplane
{
	int x;
	int y;
	int a;
	int cnt;
	int hs;
	int vs;
	u8 firing;
	u8 reload;
	u8 firerate;
	u8 tile_entry;
	int vx;
	int vy;
	u8 blastfire;
	u8 active;
	u8 firing_m;
	u8 mslot;
	u8 missiles;
	//u8 nbombs;
	u8 power;
	u16 invun;
	collbox cb;
	u8 shot;
	u8 mshot;
	u16 power_time;
	u8	shadow_flicker;
	
	u8	firing_b;
	u8	reload_b;
	u8	b_shot;
	u32	score;
} plane;

void planeSetup( void );
void planeReset( plane* p, int x, int slave );
void planeUpdate( plane* p );
void planeDraw( plane* p, int affindex, int palette );

void planeSteerLeft( plane* p );

void planeSteerRight( plane* p );

void planeAccel( plane* p );
void planeSlow( plane* p );

void planeShoot( plane* p );
void planeFireMissile( plane* p );

extern plane p1;
extern plane p2;

void planeMakeData( plane* p, playerdata* pd );
void planeReadData( plane* p, playerdata* pd );

void planeFireMissileInternal( plane* p );

void planePowerup( plane* p );

void planeDropBomb( plane* p );

#define POWER_MAX 3

#endif
