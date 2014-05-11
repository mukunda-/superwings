// helicopter :)

#include <gba.h>
#include <stdlib.h>
#include "collision.h"
#include "heli.h"
#include "sprites.h"
#include "enemies.h"
#include <stdlib.h>
#include <string.h>
#include "multiboot.h"

#include <maxmod.h>
#include "mmsolution.h"

//#include "gfx_heli.h"
#include "tables.h"
#include "xmath.h"
#include "plane.h"
#include "bullet.h"
#include "effect.h"

#include "level.h"

#define TURN_SPEED 5

#define DELAY_BEFORE_MOVE 30
#define DELAY_BEFORE_ATTACK 60
#define FLYING_SPEED 300

#define BULLET_DAMAGE 1
#define RELOAD_TIME 20

#define MIN_TDISTANCE 60

#define AIMMODE_PLAYER		0
#define AIMMODE_ABSOLUTE	1
#define AIMMODE_ANGLE		2

#define FLASHTIME 2

#define SPRITE_PRIO 1

#define LIFE_START	20

#define POINTS 260

void heliSpawn( int x, int y, u16 script_read )
{
	// create new helicopter
	helicopter* new_heli;
	new_heli = (helicopter*)malloc( sizeof( helicopter ) );
	memset( new_heli, 0, sizeof( helicopter ) );
	
	new_heli->x = x<<7;
	new_heli->y = y<<7;
	new_heli->t_x=120;
	new_heli->t_y=20;
	new_heli->velx=0;
	new_heli->vely=0;
	new_heli->flags = EFLAG_SCRIPT|EFLAG_ACTIVE;
	new_heli->angle = 0;
	new_heli->cb.t = CTYPE_ENEMY;
	new_heli->cb.src = 0;
	
	new_heli->idle = 0;
	new_heli->life=LIFE_START;
	new_heli->flash=0;
	new_heli->script_read = script_read;
	new_heli->sdelta=0;
	
	enemyRegister( (enemy*)new_heli, ETYPE_HELI );
}

int heliTurnToTarget( helicopter* h )
{
	int y,x,z;
	int wz;
	if( h->a_mode == AIMMODE_ABSOLUTE )
	{
		x = h->a_x - (h->x>>7);
		y = h->a_y - (h->y>>7);
	}
	else if( h->a_mode == AIMMODE_PLAYER )
	{
		x = ((p1.x>>8)+12) - (h->x>>7);
		y = ((p1.y>>8)+12) - (h->y>>7);
	}
	else
	{
		z = h->a_x;
		goto happyness;
	}
	
	z = (ArcTan2( x,y )+((64)<<8))>>8;
happyness:

	wz = z-h->angle;
	wz &= 255;
	if( wz > 128 ) wz = -(256-wz);
	if( wz > -TURN_SPEED && wz < TURN_SPEED )
	{
		h->angle = z;
		return 0;
	}
	else
	{
		if( wz < 0 ) h->angle -= TURN_SPEED;
		if( wz > 0 ) h->angle += TURN_SPEED;
		
		return 1;
	}
}

void heliMoveToTarget( helicopter* h )
{
	
	int x = h->t_x - (h->x>>7);
	int y = h->t_y - (h->y>>7);
	
	int zz=2;
	if( (absi(x<<7) < FLYING_SPEED*2) )
	{
		h->velx =( h->velx*190) >>8;
		zz--;
	}
	if( (absi(y<<7) < FLYING_SPEED*2 ) )
	{
		h->vely = (h->vely*190) >>8;
		zz--;
	}
	if( !zz )
		return;
	
	int z = ((ArcTan2( x,y)+(64<<8))>>8)&255;
	int c,s;
	c = (-sin_tab[(z+128)&255]>>11);//*FLYING_SPEED)>>14;
	s = (-sin_tab[(z+64)&255]>>11);//*FLYING_SPEED)>>14;
	h->velx += c;//((h->vely<<2) + (s*12))>>4;
	h->vely += s;//((h->vely<<2) + (s*12))>>4;
	if( h->velx > FLYING_SPEED ) h->velx = FLYING_SPEED;
	if( h->vely > FLYING_SPEED ) h->vely = FLYING_SPEED;
	if( h->velx < -FLYING_SPEED ) h->velx = -FLYING_SPEED;
	if( h->vely < -FLYING_SPEED ) h->vely = -FLYING_SPEED;
}

void heliProcessCmd( helicopter* h )
{
	int v;
	switch( levelScript[h->script_read++] )
	{
	case OBJCMD_MOVEXY:
		v = levelScript[h->script_read++];
		v += levelScript[h->script_read++]<<8;
		v = (s16)v;
		h->t_x = (h->x>>7) + v;
		v = levelScript[h->script_read++];
		v += levelScript[h->script_read++]<<8;
		v = (s16)v;
		h->t_y = (h->y>>7) + v;
		break;
	case OBJCMD_MOVEXA:
		h->t_x = levelScript[h->script_read++];
		break;
	case OBJCMD_MOVEYA:
		h->t_y = levelScript[h->script_read++];
		break;
	case OBJCMD_MOVEXYA:
		h->t_x = levelScript[h->script_read++];
		h->t_y = levelScript[h->script_read++];
		break;
	case OBJCMD_IDLE:
		h->idle = levelScript[h->script_read++];
		break;
	case OBJCMD_FIREP:
		
		break;
	case OBJCMD_FIREA:
		{
			int angle = levelScript[h->script_read++];
			bulletFire( (h->x>>7) , (h->y>>7), -sin_tab[(angle+128)&255]>>6, -sin_tab[(angle+64)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_small );
		}
		break;
	case OBJCMD_FIREM:
		bulletFire( (h->x>>7) , (h->y>>7), -sin_tab[(h->angle+128)&255]>>6, -sin_tab[(h->angle+64)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_small );
		//mmEffectBack( mmEffect( SFX_SHOOT1 ) );
		break;
	case OBJCMD_AIMP:
		h->a_mode = AIMMODE_ABSOLUTE;
		h->a_x = levelScript[h->script_read++];
		h->a_y = levelScript[h->script_read++];
		break;
	case OBJCMD_AIMM:
		h->a_mode = AIMMODE_PLAYER;
		break;
	case OBJCMD_AIMA:
		h->a_mode = AIMMODE_ANGLE;
		h->a_x = levelScript[h->script_read++];
		break;
	case OBJCMD_HALT:
		h->flags &= ~EFLAG_SCRIPT;
		break;
	case OBJCMD_REMOVE:
		h->flags &= ~EFLAG_ACTIVE;
	}
}

void heliReadScript( helicopter* h )
{
	if( h->flags & EFLAG_SCRIPT )
	{
		h->sdelta++;
		if( h->sdelta >= levelScript[h->script_read] )
		{
			h->sdelta -= levelScript[h->script_read];
			h->script_read++;
			heliProcessCmd( h );
		}
	}
}

void heliDestroy( helicopter* h )
{
	h->flags=0;
	mmEffectPanning( effectMediumExplosion( h->x>>7, h->y>>7 ), (h->x>>7)+16);
	/*effectStart( (h->x>>7)-8+(rand()&15),(h->y>>7)-8+(rand()&15),0,0,384,5,1,6,1,0);
	effectStart( (h->x>>7)-8+(rand()&15),(h->y>>7)-8+(rand()&15),0,0,384,5,1,6,1,-rand()&63);
	effectStart( (h->x>>7)-8+(rand()&15),(h->y>>7)-8+(rand()&15),0,0,384,5,1,6,0,-rand()&63);*/
	addPoints( h->cb.src, POINTS );
}


void heliUpdate( helicopter* h )
{
	h->frame++;
	heliReadScript( h );
	heliTurnToTarget( h );
	heliMoveToTarget( h );
	
	collbox* ch;
	if( (ch = collisionCheck( (h->x>>7),( h->y>>7), CTYPE_PLAYER )) )
	{
		h->cb.damage=LIFE_START;
		collisionHit( ch, 123, CSRC_ENEMY );
	}
	
	if( h->cb.damage )
	{
		if( comm_master )
		{
			if( h->life-h->cb.damage <= 0 )
			{
				heliDestroy( h );
			}
			else
			{
				h->flash = FLASHTIME;
				h->life -= h->cb.damage;
			}
		}
		else
		{
			h->flash = FLASHTIME;
		}
		h->cb.damage = 0;
	}
	
	h->x += h->velx;
	h->y += h->vely;
	
	if( (h->y >> 7) >= 192 + 16 )
		h->flags = 0;
	
	h->cb.x = (h->x>>7)-12;
	h->cb.y = (h->y>>7)-12;
	h->cb.w = 24;
	h->cb.h = 24;
}

void heliDraw( helicopter* h )
{
	int tile = TILE_HELI;
	int s_angle =( h->angle+8 ) >> 4;
	if( s_angle > 15 ) s_angle = 0;
	sprite* s = spriteAdd(( h->x>>7)- 16, (h->y>>7)-16, tile + ((h->frame & 7) < 4 ? 4 : 0), SPRITE_SIZE_32, SPRITE_PRIO );
	spriteEnableRotation( s, s_angle, 0 );
	if( h->flash )
	{
		h->flash--;
		spriteAddPalette( s,2 );
	}
}
