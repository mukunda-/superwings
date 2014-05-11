#include <gba.h>
#include <stdlib.h>
#include "collision.h"
#include "enemies.h"
#include "powerup.h"
#include "sprites.h"
#include "plane.h"

#include <maxmod.h>
#include "mmsolution.h"

#define tileEntry 140

#define FLAG_XDIR	EFLAG_CUSTOM1
#define FLAG_YDIR	EFLAG_CUSTOM2

extern u8 P1_LIVES;
extern u8 P2_LIVES;

void powerupSpawn( int x, int y, int item )
{
	if( x < 10 ) x = 10;
	if( y < 10 ) y = 10;
	if( x > 240-10 ) x = 240-10;
	if( y > 160-10 ) y = 160-10;
	powerup* p;
	p = (powerup*)malloc( sizeof( powerup ) );
	p->x = x;
	p->y = y;
	p->flags = EFLAG_ACTIVE + FLAG_XDIR + FLAG_YDIR;
	p->cb.t = CTYPE_POWERUP;
	p->item = item;
	enemyRegister( (enemy*)p, ETYPE_POWERUP );
}

void powerupUpdate( powerup* p ) 
{
	if( p->flags & FLAG_XDIR )
	{
		p->x++;
		if( p->x >= 240-8-2 )
			p->flags &= ~FLAG_XDIR;
	}
	else
	{
		p->x--;
		if( p->x < 8+2 )
			p->flags |= FLAG_XDIR;
	}
	if( p->flags & FLAG_YDIR )
	{
		p->y++;
		if( p->y >= 160-40-2 )
			p->flags &= ~FLAG_YDIR;
	}
	else
	{
		p->y--;
		if( p->y < 8+2 )
			p->flags |= FLAG_YDIR;
	}
	p->flash+=6;
	collbox* cb;
	
	// check collision with player
	if( (cb=collisionCheck( p->x, p->y, CTYPE_PLAYER )) )
	{
		plane* hit=0;
		if( cb == &p1.cb )
		{
			hit = &p1;
		}
		else if( cb == &p2.cb )
		{
			hit = &p2;
		}
		
		if( hit )
		{
			switch( p->item )
			{
				case POWERUP_POWER:
					planePowerup( hit );
					break;
//				case POWERUP_A:
//					hit->invun = 500;
//					break;
				case POWERUP_BOMB:
					hit->missiles++;
					break;
				case POWERUP_1UP:
					planePowerup( hit );
					if( hit == &p1 )
					{
						P1_LIVES++;
						
					}
					else if( hit == &p2 )
					{
						P2_LIVES++;
					}
			}
			if( hit == &p1 )
			{
				addPoints( CSRC_P1, 999 );
			}
			else
			{
				addPoints( CSRC_P2, 999 );
			}
			p->flags=0;
			mmEffect( SFX_POWERUP );
		}
	}
	
}

void powerupDraw( powerup* p )
{
	int off = ((p->flash & 32) < 16) ? ((p->item!=POWERUP_1UP)?140 +p->item:381) : 140;
	
	spriteAdd( p->x-4, p->y-4, off, SPRITE_SIZE_8, 2 );
}
