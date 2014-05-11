// buulelts :D

#include <gba.h>
#include <stdlib.h>
#include "sprites.h"
#include "collision.h"
#include "effect.h"
#include "tables.h"
#include "level.h"
#include "bullet.h"
#include "ingame.h"
//#include "gfx_bullets.h"

#include <maxmod.h>
#include "mmsolution.h"

#define BEHAVIOR_STATIC 0
#define BEHAVIOR_SMART 1

#define BULLET_STATE_INACTIVE 0
#define BULLET_STATE_ACTIVE	1

#define FLYING_SPEED 500

#define TILE_EXPLOSION 24

//bullet*	bullets;
bullet bullets[ MAX_BULLETS ];
int next_bullet;

void bulletsInit( void )
{
	//bullets = malloc( MAX_BULLETS*sizeof( bullet ) );
	bulletsClear();
}

void bulletsClear( void )
{
	int x;
	for(x =0;x<MAX_BULLETS;x++)
	{
		bullets[x].state = BULLET_STATE_INACTIVE;
	}
	next_bullet=0;
}

void bulletFireP( int x, int y, int vx, int vy, int type, int behavior, int target, int damage, int tile, int player )
{
	bulletFire( x,y,vx,vy,type,behavior,target,damage,tile );
	int b = next_bullet-1;
	if( b < 0 ) b = MAX_BULLETS-1;
	bullets[b].src = player;
}

void bulletFire( int x, int y, int vx, int vy, int type, int behavior, int target, int damage, int tile )
{
	while( bullets[x].state && (bullets[next_bullet].behavior >= BEHAVIOR_SPEC1) )
	{
		next_bullet++;
		if( next_bullet >= MAX_BULLETS )next_bullet=0;
	}
	bullets[next_bullet].x = x<<7;
	bullets[next_bullet].y = y<<7;
	bullets[next_bullet].vx = vx;
	bullets[next_bullet].vy = vy;
	bullets[next_bullet].type = type;
	bullets[next_bullet].behavior = behavior;
	bullets[next_bullet].target = target;
	bullets[next_bullet].state= BULLET_STATE_ACTIVE;
	bullets[next_bullet].damage = damage;
	bullets[next_bullet].tile = tile;
	bullets[next_bullet].angle=0;
	bullets[next_bullet].src = CSRC_ENEMY;
	next_bullet++;
	if( next_bullet >= MAX_BULLETS )next_bullet=0;
}

void clearBullets( int x1, int y1, int x2, int y2, int type )
{
	int x;
	for( x = 0; x < MAX_BULLETS; x++ )
	{
		if( bullets[x].state )
		if( (bullets[x].x>>7) >= x1 )
		if( (bullets[x].x>>7) < x2 )
		if( (bullets[x].y>>7) >= y1 )
		if( (bullets[x].y>>7) < y2 )
		if( bullets[x].type == type )
			bullets[x].state=0;
	}
}

void bulletPullToTarget( bullet* b, int x, int y )
{
	x = x - (b->x>>7);
	y = y - (b->y>>7);
	
	int z = ((ArcTan2( x,y)+(64<<8))>>8)&255;
	//int c,s;
	b->angle=z;
/*	c = (-sin_tab[(z+128)&255]>>5);// *FLYING_SPEED)>>14;
	s = (-sin_tab[(z+64)&255]>>5);// *FLYING_SPEED)>>14;
	b->vx = ((c*4) + (b->vx*12))>>4;//((h->vely<<2) + (s*12))>>4;
	b->vy = ((s*4) + (b->vy*12))>>4;//((h->vely<<2) + (s*12))>>4;
	if( b->vx > FLYING_SPEED ) b->vx = FLYING_SPEED;
	if( b->vy > FLYING_SPEED ) b->vy = FLYING_SPEED;
	if( b->vx < -FLYING_SPEED ) b->vy = -FLYING_SPEED;
	if( b->vy < -FLYING_SPEED ) b->vy = -FLYING_SPEED;*/
}

void bulletThrust( bullet* b )
{
	int z = b->angle;
	int c,s;
	c = (-sin_tab[(z+128)&255]>>5);//*FLYING_SPEED)>>14;
	s = (-sin_tab[(z+64)&255]>>5);//*FLYING_SPEED)>>14;
	b->vx = ((c*4) + (b->vx*12))>>4;//((h->vely<<2) + (s*12))>>4;
	b->vy = ((s*4) + (b->vy*12))>>4;//((h->vely<<2) + (s*12))>>4;
	if( b->vx > FLYING_SPEED ) b->vx = FLYING_SPEED;
	if( b->vy > FLYING_SPEED ) b->vy = FLYING_SPEED;
	if( b->vx < -FLYING_SPEED ) b->vy = -FLYING_SPEED;
	if( b->vy < -FLYING_SPEED ) b->vy = -FLYING_SPEED;
}

void bulletsRefresh( void )
{
	int x;
	collbox* ch;
	for( x = 0; x < MAX_BULLETS; x++ )
	{
		if( bullets[x].state )
		{
			//effectStart( bullets[x].x>>7,bullets[x].y>>7,0,0,448,1,0,5,0);
			if( bullets[x].behavior < BEHAVIOR_SPEC1 )
			{
				if( (ch = collisionCheck( bullets[x].x>>7, bullets[x].y>>7, bullets[x].type )) )
				{
					//effectStart( bullets[x].x>>7,bullets[x].y>>7,0,0,384,1,1,6,0);
					collisionHit( ch, bullets[x].damage, bullets[x].src );
					bullets[x].state= BULLET_STATE_INACTIVE;
				}
			}
			if( bullets[x].behavior == BEHAVIOR_SMART )
			{
				// smoke :)
				effectStart( bullets[x].x>>7,bullets[x].y>>7,0,-level_scroll_speed,effect_smoke_start,13,0,effect_smoke_len,0,0);
				if( (ch = collisionFind( (bullets[x].x>>7) - 110, (bullets[x].y>>7) - 110, (bullets[x].x>>7) + 110, (bullets[x].y>>7) + 110, bullets[x].type )) )
				{
					bulletPullToTarget( &bullets[x], ch->x+(ch->w>>1),ch->y+(ch->h>>1));
				}
				else
				{
					
				}
				bulletThrust( bullets+x );
			}
			else if( bullets[x].behavior == BEHAVIOR_SPEC1 )
			{
				if( !(bullets[x].damage & 3) )
					effectStart( (bullets[x].x>>7),(bullets[x].y>>7)+3,0,-level_scroll_speed,effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 0 );

				bullets[x].vy-=10;
				bullets[x].damage--;
				if( bullets[x].damage == 0 )
				{
					bullets[x].behavior = BEHAVIOR_EXPLOSION;
					mm_sfxhand s;
					s = mmEffect( SFX_EXPLOSION1 );
					mmEffectFrequency( s, 12000 );
					s = mmEffect( SFX_EXPLOSION1 );
					mmEffectFrequency( s, 7000 );
					mmEffectPanning( s, 200 );
					
					s = mmEffect( SFX_EXPLOSION1 );
					mmEffectFrequency( s, 6000 );
					mmEffectPanning( s, 40 );
					flashScreen( 12 );
					if( bullets[x].target != 255 )
					{
						bullets[x].vx=0;
						bullets[x].vy=0;
					}
					bullets[x].damage = 120;
				}
			}
			else if( bullets[x].behavior == BEHAVIOR_EXPLOSION )
			{
				if( bullets[x].damage > 80 )
				{
					bullets[x].target++;
					if( bullets[x].target > 3 ) bullets[x].target = 1;
				}
				else if( bullets[x].damage > 40 )
				{
					bullets[x].target++;
					if( bullets[x].target > 2 ) bullets[x].target = 1;
				}
				else
				{
					bullets[x].target++;
					if( bullets[x].target > 2 ) bullets[x].target = 0;
				}
				bullets[x].damage--;
				if( bullets[x].damage == 0 )
				{
					bullets[x].state=0;
				}
				else
				{
					if( bullets[x].target > 1 )
					{
						collisionDamageArea( (bullets[x].x>>7) - 64, (bullets[x].y>>7) - 64, (bullets[x].x>>7) + 64, (bullets[x].y>>7) +64, CTYPE_ENEMY, 7, bullets[x].src );
						clearBullets( (bullets[x].x>>7) - 64, (bullets[x].y>>7) - 64, (bullets[x].x>>7) + 64, (bullets[x].y>>7) +64, CTYPE_PLAYER );
					}
				}
			}
			bullets[x].x += (bullets[x].vx) ;
			if( bullets[x].x < (-40<<7) ) bullets[x].state = BULLET_STATE_INACTIVE;
			if( bullets[x].x > (250<<7) ) bullets[x].state = BULLET_STATE_INACTIVE;
			bullets[x].y += (bullets[x].vy);
			if( bullets[x].y < (-40<<7) ) bullets[x].state = BULLET_STATE_INACTIVE;
			if( bullets[x].y > (170<<7) ) bullets[x].state = BULLET_STATE_INACTIVE;
			bullets[x].blink++;
			bullets[x].blink &= 7;
		}
	}
}

void bulletsDraw( void )
{
	int x;
	for( x = 0; x < MAX_BULLETS; x++ )
	{
		if( bullets[x].state )
		{
			switch( bullets[x].behavior )
			{
			case BEHAVIOR_SMART:
			{
				sprite* s = spriteAdd( (bullets[x].x>>7) - 4, (bullets[x].y>>7) - 4, bullet_tile_entry+bullets[x].tile, SPRITE_SIZE_8, 1 );
				//spriteSetPriority( s=spriteAdd8( (bullets[x].x>>7) - 4, (bullets[x].y>>7) - 4, bullet_tile+bullets[x].tile ), 1 );
				if( bullets[x].angle )
				{
					int s_angle = ((bullets[x].angle+8)>>4)&15;
					spriteEnableRotation( s, s_angle, 0 );
				}
			} 
				break;
			case BEHAVIOR_STATIC:
			case BEHAVIOR_SPEC1:
				{
				int offset=0;
				if( bullets[x].tile <= 1 )
					offset = bullets[x].blink >= 4 ? 32 : 0;
				sprite* s = spriteAdd( (bullets[x].x>>7) - 4, (bullets[x].y>>7) - 4, bullet_tile_entry+bullets[x].tile+offset, SPRITE_SIZE_8, 1 );
				if( bullets[x].tile > 1 )
					spriteTall( s );
				//spriteSetPriority( s=spriteAdd16x8( (bullets[x].x>>7) - 4, (bullets[x].y>>7) - 4, bullet_tile+bullets[x].tile ), 1 );
				if( bullets[x].angle )
				{
					int s_angle = ((bullets[x].angle+8)>>4)&15;
					spriteEnableRotation( s, s_angle, 0 );
				}
				} break;
			case BEHAVIOR_EXPLOSION:
				{
				if( bullets[x].target>1 )
				{
					sprite* s = spriteAdd( (bullets[x].x>>7) - 64, (bullets[x].y>>7) - 64, TILE_EXPLOSION, SPRITE_SIZE_64, 2 );
					spriteEnableRotation( s, 17, 1 );
				}
				} break;
			}
		}
	}
}
