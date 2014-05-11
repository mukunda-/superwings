#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include "collision.h"
#include "lilplane.h"
#include "plane.h"
#include "bullet.h"
#include "effect.h"
#include "enemies.h"
#include "multiboot.h"

#include <maxmod.h>
#include "mmsolution.h"

#define spriteEntry ((32*4)+(72/8))

#define CRASH_DAMAGE 10

#define PRIO 1

#define POINTS 131

void lpSpawn( int x, int y, u16 script_read )
{
	lilplane* p;
	p = (lilplane*)malloc( sizeof( lilplane ) );
	memset( p, 0, sizeof( lilplane ) );
	p->x 			= x<<7;
	p->y 			= y<<7;
	p->flags		= EFLAG_ACTIVE + EFLAG_SCRIPT;
	p->script_read 	= script_read;
	p->cb.t			= CTYPE_ENEMY;
	p->cb.src		= 0;
	
	enemyRegister( (enemy*)p, ETYPE_JET );
}

void lpDestroy( lilplane* p )
{
	p->flags = 0;
	mm_sfxhand s = mmEffect( SFX_EXPLOSION1 );
	mmEffectVolume( s, 200 );
	mmEffectPanning( s, (p->x>>7)+16 );
	effectStart( (p->x>>7),p->y>>7,0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len ,1,0);
	addPoints( p->cb.src, POINTS );
}

void lpUpdate( lilplane* p )
{
	collbox* ch;
	if( (ch = collisionCheck( p->x>>7, p->y>>7, CTYPE_PLAYER )) )
	{
		p->cb.damage=100;
		collisionHit( ch, CRASH_DAMAGE, CSRC_ENEMY );
	}
	p->cb.x = (p->x>>7)-10;
	p->cb.y = (p->y>>7)-10;
	p->cb.w = 20;
	p->cb.h = 20;
	if( comm_master )
	{
		if( p->cb.damage )
		{
			lpDestroy( p );
		}
	}
	p->y += 300;
	if( p->y >= 170<<7 )
		p->flags=0;
}

void lpDraw( lilplane* p )
{
	if( p->flags & EFLAG_ACTIVE )
	{
		// draw
		spriteAdd( (p->x>>7)-8, (p->y>>7)-8, spriteEntry, SPRITE_SIZE_16, PRIO );
		spriteAdd( (p->x>>7)-8+6,(p->y>>7)-8+16,spriteEntry+2, SPRITE_SIZE_8, PRIO );
	}
}
