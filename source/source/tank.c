#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "collision.h"
#include "sprites.h"
#include "tank.h"
#include "level.h"
#include "effect.h"
#include "plane.h"
#include "bullet.h"
#include "tables.h"
#include "enemies.h"
#include "multiboot.h"

#include "maxmod.h"
#include "mmsolution.h"

#define FLAG_DESTROYED EFLAG_CUSTOM1

#define TILE_TANK 192
#define tile_destroyed 16

#define AIMMODE_PLAYER 0
#define AIMMODE_TARGET 1
#define AIMMODE_ANGLE 2

#define TURN_SPEED 5

#define BULLET_DAMAGE 10

#define FLASHTIME 2

#define MOVESPEED	128		// straight

#define PRIO 2

#define POINTS 650

void tankSpawn( int x, int y, u16 script_read )
{
	tank* t;
	t = (tank*)malloc( sizeof( tank ) );
	memset( t, 0, sizeof( tank ) );
	t->x = x<<7;
	t->y = (y<<7) + ((scroll_pos>>1)&127);
	t->flags = EFLAG_ACTIVE+EFLAG_SCRIPT;
	t->cb.t=CTYPE_ENEMY;
	t->cb.src=0;

	t->angle=0;
	t->script_read = script_read;
	t->sdelta=0;
	t->life=50;
	t->flash=0;
	t->moving=0;
	
	enemyRegister( (enemy*)t, ETYPE_TANK );
}

int tankAimAtTarget( tank* t )
{
	int y,x,z;
	int wz;
	if( t->a_mode == AIMMODE_TARGET )
	{
		x = t->a_x - (t->x>>7);
		y = t->a_y - (t->y>>7);
	}
	else if( t->a_mode == AIMMODE_PLAYER )
	{
		x = ((p1.x>>8)+12) - (t->x>>7);
		y = ((p1.y>>8)+12) - (t->y>>7);
	}
	else // if( t->a_mode == AIMMODE_ANGLE )
	{
		z = t->a_x;
		goto foobar;
	}
	z = (ArcTan2( x,y )+((64)<<8))>>8;
foobar:
	wz = z - t->t_angle;
	wz &= 255;
	if( wz > 128 ) wz = -(256-wz);
	if( wz > -TURN_SPEED && wz < TURN_SPEED )
	{
		t->t_angle = z;
		return 0;
	}
	else
	{
		if( wz < 0 ) t->t_angle -= TURN_SPEED;
		if( wz > 0 ) t->t_angle += TURN_SPEED;
		return 1;
	}
}

void tankProcessCmd( tank* t )
{
	switch( levelScript[ t->script_read++ ] )
	{
	case OBJCMD_AIMM:
		t->a_mode = AIMMODE_PLAYER;
		break;
	case OBJCMD_AIMP:
		t->a_mode = AIMMODE_TARGET;
		t->a_x = levelScript[t->script_read++];
		t->a_y = levelScript[t->script_read++];
		break;
	case OBJCMD_AIMA:
		t->a_mode = AIMMODE_ANGLE;
		t->a_x = levelScript[t->script_read++];
		break;
	case OBJCMD_FIREM:
		bulletFire( (t->x>>7) , (t->y>>7), -sin_tab[(t->t_angle+128)&255]>>6, -sin_tab[(t->t_angle+64)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_small );
		break;
	case OBJCMD_MOVE:
		t->angle = levelScript[t->script_read++];
		t->moving = levelScript[t->script_read++];
		
		break;
	case OBJCMD_HALT:
		t->flags &= ~EFLAG_SCRIPT;
	}
}

void tankReadScript( tank* t  )
{
	if( t->flags & EFLAG_SCRIPT )
	{
		t->sdelta++;
		if( t->sdelta >= levelScript[ t->script_read ] )
		{
			t->sdelta=0;
			t->script_read++;
			tankProcessCmd( t ) ;
		}
	}
}

void tankDestroy( tank* t )
{
	t->flags |= FLAG_DESTROYED;
	mmEffect( SFX_EXPLOSION1 );
	effectStart( (t->x>>7)-8+(rand()&15),(t->y>>7)-8+(rand()&15),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0);
	effectStart( (t->x>>7)-8+(rand()&15),(t->y>>7)-8+(rand()&15),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,-rand()&63);
	effectStart( (t->x>>7)-8+(rand()&15),(t->y>>7)-8+(rand()&15),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,-rand()&63);
	addPoints( t->cb.src, POINTS );
}

void tankUpdate( tank* t )
{
	t->y += level_scroll_speed;
	if( t->y > ((160+13)<<7) )
	{
		t->flags= 0 ;
	}
	else if( !(t->flags & FLAG_DESTROYED) )
	{
		tankReadScript( t );
		tankAimAtTarget( t );
		if( t->moving )
		{
			t->x -= (sin_tab[((t->angle+4)&7) <<5]*MOVESPEED)>>14;
			t->y -= (sin_tab[((t->angle+2)&7) <<5]*MOVESPEED)>>14;
			t->moving--;
		}
		t->cb.x = (t->x>>7)-10;
		t->cb.y = (t->y>>7)-10;
		t->cb.w = 20;
		t->cb.h = 20;
		if( t->cb.damage )
		{
			if( comm_master )
			{
				if( (t->life-t->cb.damage) <= 0 )
				{
					tankDestroy( t );
				}
				else
				{
					t->life -= t->cb.damage;
					t->flash	= FLASHTIME;
				}
			}
			else
			{
				t->flash = FLASHTIME;
			}
			t->cb.damage= 0;
		}
		
	}
	else
	{
		t->cb.w = 0;
	}
}

void tankDraw( tank* t )
{
	if( !(t->flags & FLAG_DESTROYED) )
	{
		int hf,vf;
		int frame;
		switch( t->angle&7 )
		{
			case 0:
			default:
				hf=vf=0;
				frame=0;
				break;
			case 1:
				hf=vf=0;
				frame=1;
				break;
			case 2:
				hf=vf=0;
				frame=2;
				break;
			case 3:
				hf=0;
				vf=1;
				frame=1;
				break;
			case 4:
				hf=0;
				vf=1;
				frame=0;
				break;
			case 5:
				hf=1;
				vf=1;
				frame=1;
				break;
			case 6:
				hf=1;
				vf=0;
				frame=2;
				break;
			case 7:
				hf=1;
				vf=0;
				frame=1;
		}
		
		sprite* s = spriteAdd( (t->x>>7)-16, (t->y>>7)-16, TILE_TANK+12, SPRITE_SIZE_32, PRIO );
		int s_angle = ((t->t_angle + 8) >> 4)&15;
		spriteEnableRotation( s, s_angle, 0 );
		
		if( t->flash )
		{
			spriteAddPalette( s, 3 );
		}
		
		s = spriteAdd( (t->x>>7)-16, (t->y>>7)-16, TILE_TANK+frame*4, SPRITE_SIZE_32, PRIO );
		if( hf )
			spriteFlipX( s );
		if( vf )
			spriteFlipY( s );
		if( t->flash )
		{
			t->flash--;
			spriteAddPalette( s, 2 );
		}
	}
	else
	{
		spriteAdd( (t->x>>7)-16, (t->y>>7)-16, TILE_TANK+tile_destroyed, SPRITE_SIZE_32, 3 );
	}
}
