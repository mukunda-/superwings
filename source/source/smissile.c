#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include "enemies.h"
#include "collision.h"
#include "smissile.h"
#include "effect.h"
#include "level.h"

#define FLAG_RIGHT	EFLAG_CUSTOM1
#define FLAG_LEFT	EFLAG_CUSTOM2
//#define FLAG_FLASH	EFLAG_CUSTOM3
//#define FLAG_FLASH2	EFLAG_CUSTOM4
#define FLAG_SMOKE	EFLAG_CUSTOM5

#define STAGE_FIRING	0
#define STAGE_FALLING	1

#define FIRE_SPEED	200
#define FALL_SPEED	200

#define XVEL 30

#define TILE	(120/8+32*4)

#define POINTS 63

void smissileSpawn( int x, int y )
{
	smissile* s;
	s = (smissile*)malloc( sizeof( smissile ) );
	memset( s, 0, sizeof( smissile ) );
	s->x = x<<6;
	s->y = y<<6;
	s->stage = STAGE_FIRING;
	s->flags = EFLAG_ACTIVE + ((rand()&3)*FLAG_RIGHT);
	s->cb.t = CTYPE_ENEMY;
	s->cb.src = 0;
	enemyRegister( (enemy*)s, ETYPE_SMISSILE );
	s->vel = 0;
}

void smissileUpdate( smissile* s )
{
	switch( s->stage )
	{
	case STAGE_FIRING:
		s->y -= FIRE_SPEED;
		if( s->flags & FLAG_SMOKE )
			effectStart( (s->x>>6)+4, (s->y>>6)+16, 0,-level_scroll_speed,effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 0 );
		s->flags ^= FLAG_SMOKE;
		if( s->y <= (-80<<6) )
		{
			s->stage = STAGE_FALLING;
			s->x = ((rand() % (240-8))) << 6;
		}
		break;
	case STAGE_FALLING:
		s->y += FALL_SPEED * (256+s->vel) >> 8;
		if( s->vel < 255 ) s->vel++;
		
		if( s->flags & FLAG_LEFT )
			s->x -= XVEL;
		if( s->flags & FLAG_RIGHT )
			s->x += XVEL;
		
		if( s->flags & FLAG_SMOKE )
			effectStart( (s->x>>6)+4, (s->y>>6), 0,-level_scroll_speed,effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 0 );
		s->flags ^= FLAG_SMOKE;
		if( s->y >= (160<<6) )
			s->flags = 0;
		s->cb.x = (s->x>>6)-1;
		s->cb.y = (s->y>>6);
		s->cb.w = 10;
		s->cb.h = 20;
		if( s->cb.damage )
		{
			effectMediumExplosionS( (s->x>>6)+4, (s->y>>6)+8 );
			s->flags = 0;
			addPoints( s->cb.src, POINTS );
		}
		collbox* cb;
		if( (cb = collisionCheck( (s->x>>6)+4, (s->y>>6)+16, CTYPE_PLAYER )) )
		{
			collisionHit( cb, 1, CSRC_ENEMY );
			s->flags = 0;
			effectMediumExplosionS( (s->x>>6)+4, (s->y>>6)+8 );
		}
		break;
	}
}

void smissileDraw( smissile* s )
{
	if( s->stage == STAGE_FIRING )
	{
		sprite *spr = spriteAdd( s->x>>6, s->y>>6, TILE, SPRITE_SIZE_8, 3 );
		spriteTall( spr );
		spriteEnableRotation( spr, 18, 0 );
	}
	else if( s->stage == STAGE_FALLING )
	{
		sprite *spr = spriteAdd( s->x>>6, s->y>>6, TILE, SPRITE_SIZE_8, 1 );
		spriteTall( spr );
		spriteFlipY( spr );
		/*if( s->flags & FLAG_FLASH )
		{
			s->flags &= ~FLAG_FLASH;
			spriteAddPalette( spr, 2 );
		}
		if( s->flags & FLAG_FLASH2 )
		{
			s->flags &= ~FLAG_FLASH2;
			spriteAddPalette( spr, 2 );
		}*/
	}

	
}
