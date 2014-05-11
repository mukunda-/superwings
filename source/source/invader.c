#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "collision.h"
#include "enemies.h"
#include "invader.h"
#include "sprites.h"
#include "bullet.h"
#include "effect.h"

#define move_speed 18

#define newshot ( 60 + rand() % 120)

#define tileEntry ((192/8)+(112*4))

void invaderSpawn( int x, int y )
{
	invader* p;
	p = (invader*)malloc( sizeof( invader ) );
	memset( p, 0, sizeof( invader ) );
	p->x = x<<5;
	p->y = y<<5;
	p->flags= EFLAG_ACTIVE;
	p->species = rand() % 3;
	p->hp = 1;
	p->shot = newshot;
	if( x > 120 )
	{
		p->dir = 0;
	}
	else
	{
		p->dir = 1;
	}
	p->cb.t = CTYPE_ENEMY;
	enemyRegister( (enemy*)p, ETYPE_INVADER );
}

void invaderUpdate( invader* p )
{
	if( p->dir )
	{
		p->x += move_speed;
		if( p->x >= (240+16)<<5)
		{
			p->flags=0;
		}
	}
	else
	{
		p->x -= move_speed;
		if( p->x < (-16<<5) )
		{
			p->flags = 0;
		}
	}
	p->shot--;
	if( !p->shot )
	{
		p->shot = newshot;
		bulletFire( (p->x>>5), (p->y>>5)+16, 0, 256, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_p1 );
	}
	p->frame++;
	p->cb.x = (p->x>>5) - 8;
	p->cb.y = (p->y>>5) - 8;
	p->cb.w = 16;
	p->cb.h = 16;
	if( p->cb.damage )
	{
		p->flags = 0;
		effectMediumExplosionS( (p->x>>5), (p->y>>5) );
		addPoints( p->cb.src, 123 );
	}
	collbox* cb;
	cb = collisionCheck( p->x>>5, p->y>>5, CTYPE_PLAYER );
	if( cb )
	{
		collisionHit( cb, 1, CSRC_ENEMY );
	}
}

void invaderDraw( invader* p )
{
	spriteAdd( (p->x>>5) - 8, (p->y>>5) - 8, tileEntry + (p->species<<1) + ((p->frame&15)>7?64:0), SPRITE_SIZE_16, 2 );
}
