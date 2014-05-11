#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "enemies.h"
#include "sprites.h"
#include "collision.h"
#include "boss_prototype.h"
#include "level.h"
#include "effect.h"
#include "bullet.h"
#include "tables.h"
#include "smissile.h"
#include "ingame.h"
#include "gfx_cats.h"

#include <maxmod.h>
#include "mmsolution.h"

#define xy2tile(x,y) (((x)>>3)+((y)<<2))
#define tileEntry xy2tile( 0, 144 )
#define prel(x,y) px+(x-32)*2, py+(y-16)*2
#define prelr(x,y) px-(x-32)*2, py+(y-16)*2

#define FLASHTIME 2

#define HP_LEFT		2000
#define HP_MIDDLE	4200
#define HP_RIGHT	2000

#define POINTS_LEFT 19500
#define POINTS 45500
#define POINTS_RIGHT 19500

#define STAGE_APPEAR 0
#define STAGE_CONFRONT  1
#define STAGE_NORMAL 2
#define STAGE_DEATH 3
#define STAGE_REMOVE 4

#define normal_center_y	((160/2)-20)
#define normal_center_x ((240/2))

#define death_velocity 300

#define appear_speed 500
#define confront_speed 100

extern u8 song_messages[];

void prototypeSpawn()
{	
	// copy graphics
	LZ77UnCompVram( (void*)gfx_catsTiles, (void*)0x6014800 );
	
	boss_prototype* p;
	p = (boss_prototype*)malloc( sizeof( boss_prototype ) );
	memset( p, 0, sizeof( boss_prototype ) );
	
	p->x = 120<<8;//x<<8;
	p->y = 200<<8;//y<<8;
	p->cb_m.t = CTYPE_ENEMY;
	p->cb_l.t = CTYPE_ENEMY;
	p->cb_r.t = CTYPE_ENEMY;
	p->flags = EFLAG_SCRIPT | EFLAG_ACTIVE;
	p->scale = 65536;
	enemyRegister( (enemy*)p, ETYPE_BOSS_PROTOTYPE );
	collisionRegister( &p->cb_l );
	collisionRegister( &p->cb_r );
	p->hp_left = HP_LEFT;
	p->hp_right = HP_RIGHT;
	p->hp_middle = HP_MIDDLE;
	
	p->stage = STAGE_APPEAR;
	mm_sfxhand s = mmEffect( SFX_MISSILE );
	mmEffectFrequency( s, 3000 );
	flashScreen(20);
	
	p->blaster1=0;
	p->blaster2=0;
	p->blaster3=0;
}

void prototypeUnload( boss_prototype* p )
{
	if( p->flags & EFLAG_ACTIVE )
	{
		collisionUnregister( &p->cb_l );
		collisionUnregister( &p->cb_r );
		p->flags=0;
	}
}

int checkDamage( collbox* c, int life, u8* flashv )
{
	if( c->damage )
	{
		if( life - c->damage < 0 )
		{
			life = 0;
		}
		else
		{
			life -= c->damage;
			*flashv = FLASHTIME;
		}
		c->damage =0;
	}
	return life;
}

void prototypeDestroyLeftWing( boss_prototype* p )
{
	effectMediumExplosion( (p->x>>8)-40, (p->y>>8)+7 );
	p->hp_left=0;
}

void prototypeDestroyRightWing( boss_prototype* p )
{
	effectMediumExplosion( (p->x>>8)+40, (p->y>>8)+7 );
	p->hp_right=0;
}

void damagePoint( int x ,int y )
{
	collbox* cb;
	cb = collisionCheck( x, y, CTYPE_PLAYER );
	if( cb )
	{
		collisionHit( cb, 1, CSRC_ENEMY );
	}
}

void prototypeUpdate( boss_prototype* p )
{
	switch( p->stage )
	{
	case STAGE_APPEAR:
		
		p->y -= appear_speed;
		if( p->y <= (-64<<8) )
		{
			p->scale = (int)(65536*1.8);
			p->y = (-64<<8);
			p->stage = STAGE_CONFRONT;
		}
		break;
	case STAGE_CONFRONT:
		level_scroll_speed+=2;
		p->y += confront_speed;
		if( p->y >= 0 )
		{
			p->stage = STAGE_NORMAL;
		}
		break;
	case STAGE_NORMAL:
		if( p->slen )
		{
			p->x -= sin_tab[(p->sdir+128)&255]>>6;
			p->y -= sin_tab[(p->sdir+64)&255]>>6;
			p->scale += p->zdir;
			if( p->scale > 65536*2 ) p->scale = 65536*2;
			p->slen--;
		}
		else
		{
			p->slen = rand() & 127;
			if( (p->y>>8) < normal_center_y )
			{
				if( (p->x>>8) < normal_center_x )
				{
					p->sdir = (rand() & 63) + 64;
				}
				else
				{
					p->sdir = (rand() & 63) + 128;
				}
			}
			else
			{
				if( (p->x>>8) < normal_center_x )
				{
					p->sdir = (rand() & 63);
				}
				else
				{
					p->sdir = (rand() & 63) + 192;
				}
			}
			if( p->scale >= 65536*2 )
			{
				p->zdir = -(rand() &255);
			}
			else
			{
				p->zdir = (rand() &255);
			}
				
		}
		break;
	case STAGE_DEATH:
		p->y += death_velocity;
		/*if( p->blaster1 == 0 )
		{
			effectStart( (p->x>>8)-40 + (rand() % 80), (p->y>>8)-4 + (rand() % 16), 0,127,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,0 );
			p->blaster1 = 1+(rand() % 1);
		}
		else
		{
			p->blaster1--;
		}*/
		if( p->y >= ((160+40)<<8) )
		{
			p->flags = 0;
			collisionUnregister( &p->cb_l );
			collisionUnregister( &p->cb_r );
			p->stage = STAGE_REMOVE;
		}
		break;
	}
	int pw;
	pw = (32*p->scale)>>16;
//	if( p->scale < 65536*2 )
//		p->scale += 100;
		
	int px,py;
	px = p->x>>8;
	py = p->y>>8;
	p->cb_l.x = px - ((32*p->scale)>>16);
	p->cb_l.y = py;// - ((16*p->scale)>>16);
	p->cb_l.w = pw+1;
	p->cb_l.h = pw>>1;
	
	p->cb_m.x = px - ((8*p->scale)>>16);
	p->cb_m.y = py - ((16*p->scale)>>16);
	p->cb_m.w = pw>>1;
	p->cb_m.h = pw;
	
	p->cb_r.x = px + (((7)*p->scale)>>16);
	p->cb_r.y = py;// - ((16*p->scale)>>16);
	p->cb_r.w = pw;
	p->cb_r.h = pw>>1;
	
	
	if( p->stage >= STAGE_CONFRONT )
	{
		if( p->hp_left )
		{
			p->hp_left = checkDamage( &p->cb_l, p->hp_left, &p->flash_l );
			if( !p->hp_left )
			{
				prototypeDestroyLeftWing( p );
				addPoints( p->cb_l.src, POINTS_LEFT );
			}
			else
			{
				damagePoint( prel( 5,28 ) );
				damagePoint( prel( 11,24 ) );
				damagePoint( prel( 17,20 ) );
				damagePoint( prel( 27,13 ) );
				damagePoint( prel( 27,22 ) );
				damagePoint( prel( 27,33 ) );
			}
		}
		if( p->hp_middle )
		{
			p->hp_middle = checkDamage( &p->cb_m, p->hp_middle, &p->flash_m );
			if( !p->hp_middle )
			{
				if( p->hp_left )
					prototypeDestroyLeftWing( p );
				if( p->hp_right )
					prototypeDestroyRightWing( p );
				addPoints(  p->cb_m.src, POINTS );
				p->stage= STAGE_DEATH;
				bulletFire( p->x>>8,p->y>>8,0,death_velocity>>1,CTYPE_ENEMY,BEHAVIOR_SPEC1,255,1,3);
				levelEnd();
			}
			else
			{
				damagePoint( prel( 40, 6 ) );
				damagePoint( prel( 40,25 ) );
			}
		}
		if( p->hp_right )
		{
			p->hp_right = checkDamage( &p->cb_r, p->hp_right, &p->flash_r );
			if( !p->hp_right )
			{
				prototypeDestroyRightWing( p );
				addPoints( p->cb_r.src, POINTS_RIGHT );
			}
			else
			{
				damagePoint( prelr( 5,28 ) );
				damagePoint( prelr( 11,24 ) );
				damagePoint( prelr( 17,20 ) );
				damagePoint( prelr( 27,13 ) );
				damagePoint( prelr( 27,22 ) );
				damagePoint( prelr( 27,33 ) );
			}
		}
	}
	p->blaster1++;
	if( p->blaster1 == 5 )
	{
		p->blaster1=0;
		if( p->stage < STAGE_CONFRONT )
		{
			effectStart( px-12, py+4, 0,127,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,0 );
			effectStart( px+12, py+4, 0,127,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,0 );
		}
		else
		{
			if( p->hp_left )
				effectStart( px-24, py+8, -sin_tab[(p->sdir+128)&255]>>7,70,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,20 );
			if( p->hp_right )
				effectStart( px+24, py+8, -sin_tab[(p->sdir+128)&255]>>7,70,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,20 );
		}
	}
	if( !p->hp_left )
	{
		p->blaster2++;
		if( p->blaster2 >= 18 )
		{
			effectStart( (p->x>>8)-50 + (rand() % 30), (p->y>>8)-8 + (rand() % 20), 0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0 );
			p->blaster2 = rand() & 3;
		}
	}
	if( !p->hp_right )
	{
		p->blaster3++;
		if( p->blaster3 >= 18 )
		{
			effectStart( (p->x>>8)+50 - (rand() % 30), (p->y>>8)-8 + (rand() % 20), 0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0 );
			p->blaster3 = rand() & 3;
		}
	}
	
	if( p->stage >= STAGE_CONFRONT )
	{
		if( song_messages[1] )
		{
			song_messages[1] = 0;
			if( p->hp_middle )
			{
				bulletFire( px, py, -200,300,CTYPE_PLAYER, 	BEHAVIOR_STATIC,0,1,bullet_tile_p3 );
				bulletFire( px, py, 0,300,CTYPE_PLAYER, 	BEHAVIOR_STATIC,0,1,bullet_tile_p3 );
				bulletFire( px, py, 200,300,CTYPE_PLAYER, 	BEHAVIOR_STATIC,0,1,bullet_tile_p3 );
				bulletFire( px, py, 300, 0,CTYPE_PLAYER, 	BEHAVIOR_STATIC,0,1,bullet_tile_large );
				bulletFire( px, py, -300,0,CTYPE_PLAYER, 	BEHAVIOR_STATIC,0,1,bullet_tile_large );
			}
		}
		if( song_messages[2] )
		{
			song_messages[2] = 0;
			if( p->hp_middle )
			{
				smissileSpawn(px - 30, py+8 );
				smissileSpawn(px + 30, py+8 );
				mmEffect( SFX_MISSILE );
			}
		}
	}
}

void prototypeDraw( boss_prototype* p )
{
	p->scale_jitter = (~p->scale_jitter)&1;
	int ox,oy;
	int scale;
	if( p->scale_jitter )
		scale = p->scale>>1;// * 256) >> 8;
	else
		scale = ((p->scale>>1) * 1000) >> 10;
	//scale =65536*1;
	
	ox = (p->x>>8);
	oy = (p->y>>8);
	
	if( oy >= -64 )
	{
		
		sprite* center;
		center = spriteAdd( ox - 16, oy - 32 , tileEntry+6 + (p->hp_middle?0:16), SPRITE_SIZE_64, (p->hp_middle?2:3) );
		spriteTall( center );
		spriteEnableRotation( center, 19, 0 );
		
		sprite* left_wing;
		left_wing = spriteAdd( ox - 64 - (((scale-65536)*32)>>16) , oy - 32 , tileEntry + (p->hp_left?0:16), SPRITE_SIZE_64, p->hp_middle?2:3);
		spriteEnableRotation( left_wing, 19, 0 );
		
		sprite* right_wing;
		right_wing = spriteAdd( ox + (((scale-65536)*32)>>16) , oy-32, tileEntry+8 + (p->hp_right?0:16), SPRITE_SIZE_64, p->hp_middle?2:3 );
		spriteEnableRotation( right_wing, 19, 0 );
		
		spriteSetAffineMatrix( 19, (65536*256)/scale,0,0,(65536*256)/scale );
		
		if( p->flash_l )
		{
			p->flash_l--;
			spriteAddPalette( left_wing, 2 );
		}
		if( p->flash_m )
		{
			p->flash_m--;
			spriteAddPalette( center, 2 );
		}
		if( p->flash_r )
		{
			p->flash_r--;
			spriteAddPalette( right_wing, 2 );
		}
	}
}
