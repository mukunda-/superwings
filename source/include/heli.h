#ifndef HELI_H
#define HELI_H

#include "collision.h"

// more linked lists :)

typedef struct  thelicopter
{
	struct thelicopter* next;
	u8 flags;
	u8 type;
	u8 life;
	u8 frame;
	collbox cb;
	
	s16 x;
	s16 y;
	s16 t_x;
	s16 t_y;
	u8 a_x;
	u8 a_y;
	u8 a_mode;
	u8 angle;	// 32 = 1/8 angle
	
	s16 velx;
	s16 vely;
	
	u16 script_read;
	u8 idle;
	u8 sdelta;
	
	u8 flash;
	u8 reserved;
	u8 reserved1;
	u8 reserved2;
	
} helicopter;

#define HELI_STATE_ACTIVE 1

#define HELI_MOVET	1
#define HELI_MOVE	2
#define HELI_ATTACK 3
#define HELI_IDLE	4
#define HELI_MOVEA	5

#define TILE_HELI 8

//void heliInit( void );
void heliSpawn( int x, int y, u16 script_read );
void heliUpdate( helicopter* h );
void heliDraw( helicopter* h );

#endif
