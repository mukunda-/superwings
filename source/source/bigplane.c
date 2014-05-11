#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include "level.h"
#include "collision.h"
#include "bigplane.h"
#include "effect.h"
#include "tables.h"
#include "plane.h"
#include "bullet.h"
#include "enemies.h"
#include "multiboot.h"
#include "powerup.h"

#include <maxmod.h>
#include "mmsolution.h"

#define tileEntry	16

#define FLASHTIME	2

#define BULLET_DAMAGE 10

#define reload_time 15

#define PRIO 1

#define POINTS 3120

void bpSpawn( int x, int y, u16 script_read )
{
	bigplane* p = (bigplane*)malloc( sizeof( bigplane ) ) ;
	memset( p, 0, sizeof( bigplane ) );
	
	p->x		= x<<7;
	p->y		= y<<7;
	p->flags 	= EFLAG_ACTIVE+EFLAG_SCRIPT;
	p->cb.t 	= CTYPE_ENEMY;
	p->cb.src	=0;
	p->angle	= 0;
	p->speed	= 0;
	p->life		= 240;
	p->flash	= 0;
	
	p->fireshots=0;
	
	p->script_read = script_read;
	enemyRegister( (enemy*)p, ETYPE_BIGPLANE );
}

void bpFire( bigplane* p )
{
//	int x = ((p1.x>>8)+12) - (p->x>>7);
//	int y = ((p1.y>>8)+12) - (p->y>>7);
	//int z = (ArcTan2( x,y )+((64)<<8))>>8;
	
	p->firetmr=0;
	p->firedir = 128;
	p->fireshots=3;
}

void bpProcessCmd( bigplane* p )
{
	switch( levelScript[ p->script_read++ ] )
	{
	case OBJCMD_FIREM:
		bpFire( p );
		break;
	case OBJCMD_MOVE:
		p->angle = levelScript[p->script_read++];
		p->speed = levelScript[p->script_read++];
		break;
	case OBJCMD_HALT:
		p->flags &= ~EFLAG_SCRIPT;
		break;
	case OBJCMD_REMOVE:
		p->flags &= ~EFLAG_ACTIVE;
	}
}

void bpReadScript( bigplane* p )
{
	if( p->flags & EFLAG_SCRIPT )
	{
		p->sdelta++;
		if( p->sdelta >= levelScript[ p->script_read ] )
		{
			p->sdelta=0;
			bpProcessCmd( p );
		}
	}
}

void bpDestroy( bigplane* p )
{
	p->flags=0;
	mmEffectPanning( mmEffect( SFX_EXPLOSION1 ),(p->x>>7)+16) ;
	effectStart( (p->x>>7)-16+(rand()&31),(p->y>>7)-16+(rand()&31),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0);
	effectStart( (p->x>>7)-16+(rand()&31),(p->y>>7)-16+(rand()&31),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,-rand()&63);
	effectStart( (p->x>>7)-16+(rand()&31),(p->y>>7)-16+(rand()&31),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,-rand()&63);
	powerupSpawn( p->x>>7, p->y>>7, POWERUP_POWER );
	addPoints( p->cb.src, POINTS );
}

void bpFrame( bigplane* p )
{
	if( p->cb.damage )
	{
		if( comm_master )
		{
			if( p->life - p->cb.damage >= 0 )
			{
				p->flash=FLASHTIME;
				p->life -= p->cb.damage;
			}
			else
			{
				bpDestroy( p );
			}
		}
		else
		{
			p->flash = FLASHTIME;
		}
		p->cb.damage=0;
	}
	bpReadScript( p );
	if( p->fireshots )
	{
		if( p->firetmr )
		{
			p->firetmr--;
		}
		else
		{
			p->fireshots--;
			p->firetmr = reload_time;
			bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128)&255]>>6, -sin_tab[(p->firedir+64)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
		//	bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128-16)&255]>>6, -sin_tab[(p->firedir+64-16)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
		//	bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128+16)&255]>>6, -sin_tab[(p->firedir+64+16)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
			bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128-25)&255]>>6, -sin_tab[(p->firedir+64-25)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
			bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128+25)&255]>>6, -sin_tab[(p->firedir+64+25)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
			bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128+64)&255]>>6, -sin_tab[(p->firedir+64+64)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
			bulletFire( (p->x>>7) , (p->y>>7), -sin_tab[(p->firedir+128-64)&255]>>6, -sin_tab[(p->firedir+64-64)&255]>>6, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, BULLET_DAMAGE, bullet_tile_large );
		}
	}
	p->x -= (sin_tab[(p->angle+128)&255]*p->speed)>>14;
	p->y -= (sin_tab[(p->angle+64)&255]*p->speed)>>14;
	p->cb.x = (p->x>>7)-27;
	p->cb.y = (p->y>>7)-24;
	p->cb.w = 54;
	p->cb.h = 24;
	p->frame++;
}

void bpDraw( bigplane* p ) 
{
	sprite *s1, *s2;
	s1 = spriteAdd( (p->x>>7)-28, (p->y>>7)-24, tileEntry, SPRITE_SIZE_64, 2 );
	s2 = spriteAdd( (p->x>>7)-28+16, (p->y>>7)-24+32, tileEntry+(4*32)+2, SPRITE_SIZE_32, 2 );
	spriteWide( s1 );
	spriteWide( s2 );
	if( (p->frame & 3) > 1 )
	{
		spriteWide( spriteAdd( (p->x>>7)-28+8, (p->y>>7)-24+8, tileEntry+32*4, SPRITE_SIZE_8, PRIO ) );
		spriteWide( spriteAdd( (p->x>>7)-28+33, (p->y>>7)-24+8, tileEntry+32*4, SPRITE_SIZE_8, PRIO ) );
	}
	
	if( p->flash )
	{
		p->flash--;
		spriteAddPalette( s1, 2 );
		spriteAddPalette( s2, 2 );
	}
}
