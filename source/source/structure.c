#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "collision.h"
#include "enemies.h"
#include "sprites.h"
#include "structure.h"
#include "level.h"
#include "effect.h"
#include "powerup.h"
#include "bullet.h"

#include <maxmod.h>
#include "mmsolution.h"

#define xy2tile(x,y) (((x)>>3)+((y)<<2))

#define FLASHTIME 2

#define FLAG_DESTROYED EFLAG_CUSTOM1

const u16 structure_tiles[] ={
xy2tile(  0, 80 ),	// warehouse
xy2tile( 160, 56 ),	// barrel
xy2tile( 176, 32 ),	// barrels
xy2tile( 168, 56 ),	// crate
xy2tile( 176, 56 ),	// crates
xy2tile( 32, 80 ),	// warehouse destroyed
xy2tile( 96, 40 ),	// small rubble
xy2tile( 104, 40 ),	// wide rubble
xy2tile( 128,48 ),  // large rubble
xy2tile( 128, 80 ), // big warehouse
xy2tile( 64,80 ), // big warehouse destroyed
};

const u8 structure_sizes[] ={
	STRUCTURE_SIZE_32x40,	// warehouse
	STRUCTURE_SIZE_8x8,		// barrel
	STRUCTURE_SIZE_16x16,	// barrels
	STRUCTURE_SIZE_8x8,		// crate
	STRUCTURE_SIZE_8x16,	// crates
	STRUCTURE_SIZE_32x40,	// destroyed warehouse
	STRUCTURE_SIZE_8x8,		// small rubble
	STRUCTURE_SIZE_8x16,	// wide rubble
	STRUCTURE_SIZE_32x32,	// large rubble
	STRUCTURE_SIZE_64x64,	// big warehouse
	STRUCTURE_SIZE_64x64,	// big warehouse destroyed
};

const u16 structure_hp[] ={
	300,	// warehouse
	10,		// barrel
	10,		// barrels
	10,		// crate
	10,		// crates
	0,		// destroyed warehouse
	0,		// small rubble
	0,		// wide rubble
	0,		// large rubble
	500,	// big warehouse
	0,		// destroyed big warehouse
};

void structureSpawn( int x, int y, int cls, u16 script )
{
	structure* s;
	s = (structure*)malloc( sizeof( structure ) );
	memset( s, 0, sizeof( structure ) );
	s->x = x;
	s->y = (y<<7) + ((scroll_pos>>1)&127);
	s->flags = EFLAG_ACTIVE + EFLAG_SCRIPT;
	s->cls = cls;
	s->hp = structure_hp[ cls ];
	s->cb.t = CTYPE_ENEMY;
	s->cb.src =0 ;
	s->script_read = script;
	enemyRegister( (enemy*)s, ETYPE_STRUCTURE );
}

void structureDestroy( structure* s )
{
	addPoints( s->cb.src, structure_hp[ s->cls ]*13 );
	switch( s->cls )
	{
	case STRUCTURE_WAREHOUSE:
		{
			mm_sfxhand sfx;
			sfx = effectMediumExplosion( (s->x) + 15, (s->y>>7) + 16 );
			mmEffectPanning( sfx, (s->x)+16);
			mmEffectFrequency( sfx, 10000 );
			effectStart( (s->x)+15, (s->y>>7)+16, 0, 0, effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 1, 0 );
			s->cls = STRUCTURE_WAREHOUSE_DESTROYED;
			if( (s->flags >>4) )
			{
				powerupSpawn( (s->x)+15, (s->y>>7)+16, s->flags >> 4 );
			}
			s->flags = EFLAG_ACTIVE+FLAG_DESTROYED;
		}
		break;
	case STRUCTURE_BIGHOUSE:
		{
			mm_sfxhand sfx;
			sfx = effectMediumExplosion( (s->x) + 15, (s->y>>7) + 16 );
			mmEffectPanning( sfx, (s->x)+16);
			mmEffectFrequency( sfx, 10000 );
			//effectStart( (s->x)+15, (s->y>>7)+16, 0, 0, effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 1, 0 );
			bulletFire( s->x + 26, (s->y>>7) + 31,0,level_scroll_speed,CTYPE_X,BEHAVIOR_SPEC1,255,1,3);
			s->cls = STRUCTURE_BIGHOUSE_DESTROYED;
			if( (s->flags >>4)  )
				powerupSpawn( (s->x)+15, (s->y>>7)+16, s->flags >> 4 );
				
			s->flags = EFLAG_ACTIVE+FLAG_DESTROYED;
		}
		break;
	case STRUCTURE_BARRELS:
		mmEffectPanning( effectMediumExplosion( (s->x)+8, (s->y>>7)+8 ), (s->x)+16);
		s->x -= 8;
		s->y -= 8<<7;
		s->cls = STRUCTURE_LARGE_RUBBLE;
		s->flags = EFLAG_ACTIVE+FLAG_DESTROYED;
		break;
	case STRUCTURE_BARREL:
	case STRUCTURE_CRATE:
		effectStart( (s->x)+4, (s->y>>7)+4, 0, 0, effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 0 );
		mmEffectPanning( mmEffect( SFX_EXPLOSION1 ), (s->x)+16 );
		s->cls = STRUCTURE_SMALL_RUBBLE;
		s->flags = EFLAG_ACTIVE+FLAG_DESTROYED;
		break;
	case STRUCTURE_CRATES:
		effectStart( (s->x)+3, (s->y>>7)+4, 0, 0, effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 0 );
		effectStart( (s->x)+8, (s->y>>7)+4, 0, 0, effect_explosion_start, effect_explosion_speed, 1, effect_explosion_len, 0, 1 );
		mmEffectPanning( mmEffect( SFX_EXPLOSION1 ), (s->x)+16 );
		s->cls = STRUCTURE_WIDE_RUBBLE;
		s->flags = EFLAG_ACTIVE+FLAG_DESTROYED;
	}
}

void structureProcessCmd( structure* s )
{
	switch( levelScript[ s->script_read++ ] )
	{
	case OBJCMD_ITEM:
		s->flags &= 0xF;
		s->flags |= levelScript[ s->script_read ]<<4;
		s->script_read++;
		break;
	case OBJCMD_HALT:
		s->flags &= ~EFLAG_SCRIPT;
	}
}

void structureReadScript( structure* s )
{
	if( s->flags & EFLAG_SCRIPT )
	{
		s->sdelta++;
		if( s->sdelta >= levelScript[ s->script_read ] )
		{
			s->script_read++;
			s->sdelta=0;
			structureProcessCmd( s );
		}
	}
}

void structureUpdate( structure* s )
{
	if( s->cb.damage )
	{
		if( s->hp - s->cb.damage <= 0 )
		{
			structureDestroy( s );
		}
		else
		{
			s->flash = FLASHTIME;
			s->hp -= s->cb.damage;
		}
		s->cb.damage = 0;
	}
	s->y += level_scroll_speed;
	if( s->y > 160<<7 )
	{
		s->flags = 0;
	}
	s->cb.x = (s->x);
	s->cb.y = (s->y >> 7);
	structureReadScript( s );
	if( !(s->flags & FLAG_DESTROYED) )
	{
		switch( structure_sizes[ s->cls ] )
		{
		case STRUCTURE_SIZE_8x8:
			s->cb.w = 8;
			s->cb.h = 8;
			break;
		case STRUCTURE_SIZE_8x16:
			s->cb.w = 16;
			s->cb.h = 8;
			break;
		case STRUCTURE_SIZE_16x16:
			s->cb.w = 16;
			s->cb.h = 16;
			break;
		case STRUCTURE_SIZE_32x32:
			s->cb.w = 32;
			s->cb.h = 32;
			break;
		case STRUCTURE_SIZE_32x40:
			s->cb.w = 32;
			s->cb.h = 40;
			break;
		case STRUCTURE_SIZE_64x64:
			s->cb.w = 64-14;
			s->cb.h = 64-8;
		}
	}
	else
	{
		s->cb.w=0;
	}
}

void structureDraw( structure* s )
{
	int size=0;
	int wide=0;
	int tall=0;
	int ext1=0;
	switch( structure_sizes[ s->cls ] )
	{
	case STRUCTURE_SIZE_8x8:
		size = SPRITE_SIZE_8;
		break;
	case STRUCTURE_SIZE_8x16:
		size = SPRITE_SIZE_8;
		wide = 1;
		break;
	case STRUCTURE_SIZE_16x16:
		size = SPRITE_SIZE_16;
		break;
	case STRUCTURE_SIZE_32x32:
		size = SPRITE_SIZE_32;
		break;
	case STRUCTURE_SIZE_32x40:
		size = SPRITE_SIZE_32;
		ext1 = 1;
		break;
	case STRUCTURE_SIZE_64x64:
		size = SPRITE_SIZE_64;
	}
	sprite* spr;
	spr = spriteAdd( s->x, s->y>>7, structure_tiles[s->cls], size, 3 );
	if( s->flash )
		spriteAddPalette( spr, 2 );
	if( wide )
		spriteWide( spr );
	if( tall )
		spriteTall( spr );
	if( ext1 )
	{
		spr = spriteAdd( s->x, (s->y>>7)+32, structure_tiles[s->cls]+128, SPRITE_SIZE_16, 3 );
		spriteWide( spr );
		if( s->flash )
		{
			spriteAddPalette( spr, 2 );
		}
	}
	if( s->flash )
		s->flash--;
}
