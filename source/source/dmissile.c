#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "enemies.h"
#include "dmissile.h"
#include "sprites.h"
#include "tables.h"
#include "bullet.h"
#include "effect.h"
#include "level.h"

#define z_start 255
#define fvel 127
#define zvel 5

#define tileEntry ((184/8)+(80*4))

#define cosine(x) ((sin_tab[((x)+64)&255]*200)>>14)
#define sine(x) ((sin_tab[(x)&255]*200)>>14)

void dmissileSpawn( int x, int y, u8 dir )
{
	dmissile* m;
	m = (dmissile*)malloc( sizeof( dmissile ) );
	
	memset( m, 0, sizeof( dmissile ) );
	
	m->x = x<<7;
	m->y = y<<7;
	m->z = z_start;
	m->dir = dir;
	m->cb.t = CTYPE_ENEMY;
	m->cb.w = 0;
	m->vel = fvel;
	m->flags= EFLAG_ACTIVE;
	enemyRegister( (enemy*)m, ETYPE_DMISSILE );
}

void dmissileUpdate( dmissile* m )
{
	int d = (m->dir - 64) & 255;
	m->x += (sin_tab[(d+64)&255] * m->vel) >> 14;
	m->y += (sin_tab[d] * m->vel) >> 14;
	
	if( m->vel < 500 )
		m->vel += 5;
	m->smoke++;
	if( m->smoke >= 3 )
	{
		m->smoke = 0;
		effectStart( (m->x>>7), (m->y>>7), 0,-level_scroll_speed,effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 0 );
	}
	if( m->z > 0 )
	{
		m->z -= zvel;
		if( m->z <= 0 )
		{
			m->z = 0;
			m->flags = 0;
			effectMediumExplosionS( m->x>>7, m->y>>7 );
			int a = m->dir;
			bulletFire( m->x>>7, m->y>>7, cosine(a),sine(a), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+32),sine(a+32), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+64),sine(a+64), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+96),sine(a+96), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+128),sine(a+128), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+160),sine(a+160), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+192),sine(a+192), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
			bulletFire( m->x>>7, m->y>>7, cosine(a+224),sine(a+224), CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_small );
		}
	}
}

void dmissileDraw( dmissile* m )
{
	sprite* s;
	s = spriteAdd( (m->x>>7)-4, (m->y>>7)-4, tileEntry + (m->z>>6), SPRITE_SIZE_8, 0 );
	spriteEnableRotation( s, ((m->dir+8)>>4)&15, 0 );
}
