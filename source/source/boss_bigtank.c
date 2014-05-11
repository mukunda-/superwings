#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include "enemies.h"
#include "boss_bigtank.h"
#include "gfx_bigtank.h"
#include "tables.h"
#include "level.h"
#include "dmissile.h"
#include "bullet.h"

#include <maxmod.h>
#include "mmsolution.h"

#define tileEntry 576

#define spr_affine_body		25
#define spr_affine_turret	26

#define HIT_POINTS 4100

#define FLASHTIME 2

#define STAGE_APPEAR 1
#define STAGE_POSITION 2
#define STAGE_ATTACK1 3
#define STAGE_MOVELEFT 4

#define FAST_SPEED 500
#define SLOW_SPEED 140

#define ACCEL 10

#define TURN_ANGLE 12
#define TURN_SPEED 50

#define TURN_SPEED_TURRET 1

#define cosine(x) sin_tab[((x)+64)&255]
#define sine(x) sin_tab[(x)&255]

void bigtankSpawn()
{
	boss_bigtank* t;
	t = (boss_bigtank*)malloc( sizeof( boss_bigtank ) );
	memset( t, 0, sizeof( boss_bigtank ) );
	t->x = 120<<8;
	t->y = -40<<8;
	t->flags = EFLAG_ACTIVE;
	t->cb.t = CTYPE_ENEMY;
	t->hp = HIT_POINTS;
	t->stage = STAGE_APPEAR;
	t->speed = level_scroll_speed<<1;
	t->t_angle = 0;
	t->flash=0;
	t->duration=0;
	t->m_speed=0;
	t->m_angle=0;
	t->t_x=0;
	t->t_y=0;
	t->s_missile=0;
	t->launch_switch=0;
	LZ77UnCompVram( (void*)gfx_bigtankTiles, (void*)0x6014800 );
	enemyRegister( (enemy*)t, ETYPE_BIGTANK );
	t->music_fade=1024;
	
}

static inline int abs_i( int a )
{
	if( a < 0 ) a = -a;
	return a;
}

int bigtankAimAtTarget( boss_bigtank* t )
{
	int y,x,z;
	int wz;
	x = t->t_x - (t->x>>8);
	y = t->t_y - (t->y>>8);
	
	z = (ArcTan2( x,y )+((64)<<8))>>8;

	wz = z-(t->t_angle);
	wz &= 255;
	if( wz > 128 ) wz = -(256-wz);
	if( wz > -TURN_SPEED_TURRET && wz < TURN_SPEED_TURRET )
	{
		t->t_angle = z;
		return 0;
	}
	else
	{
		if( wz < 0 ) t->t_angle -= TURN_SPEED_TURRET;
		if( wz > 0 ) t->t_angle += TURN_SPEED_TURRET;
		
		return 1;
	}
}

void bigtankFireMissile( boss_bigtank* t )
{
	t->launch_switch = !t->launch_switch;
	int cx = t->x>>8;
	int cy = t->y>>8;
	int ca = (t->t_angle) > 128 ? -(256-(t->t_angle)) : (t->t_angle);
	
	int horz = 15;
	if( t->launch_switch )
		horz = -horz;
	
	ca -= 64;
	ca &= 255;
	dmissileSpawn( cx + ((cosine( ca+64 ) * horz) >> 14), cy +( (sine( ca+64 ) * horz) >> 14), (ca+64)&255 );
	mm_sfxhand s = mmEffect( SFX_MISSILE );
	mmEffectFrequency( s, 10000 );
	//mmEffectVolume( s, 150 );
}

void bigtankUpdate( boss_bigtank* t )
{
	if( t->music_fade != 0 )
	{
		t->music_fade-=4;
		
		if( t->music_fade == 0 )
		{
			mmStop();
			mmVolume( 1024 );
			mmStart( MOD_BOSS1, MM_PLAY_LOOP );
		}
		else
		{
			mmVolume( t->music_fade );
		}
		return;
	}
	int cx,cy,ca,cma;
	cx=  t->x>>8;
	cy = t->y>>8;
	ca = (t->angle>>8) > 128 ? -(256-(t->angle>>8)) : (t->angle>>8);
	
	if( t->hp )
	{
		switch( t->stage )
		{
		case STAGE_APPEAR:
			level_scroll_speed++;
			t->speed++;
			if(level_scroll_speed >= 300 )
			{
				t->stage = STAGE_MOVELEFT;
	//			t->t_x = 20 + (rand() % 220);
	//			t->t_y = 20 + (rand() % 100);
			}
			break;
			
		case STAGE_MOVELEFT:
			if( t->duration )
			{
				t->duration--;
				cma = (t->m_angle) > 128 ? -(256-(t->m_angle)) : (t->m_angle);
				if( ca < cma )
				{
					t->angle += TURN_SPEED;
				}
				else if( ca > cma )
				{
					t->angle -= TURN_SPEED;
				}
				if( t->speed < t->m_speed )
					t->speed += ACCEL;
				else
					t->speed -= ACCEL;
				
			}
			else
			{
				t->duration = rand() & 127;
				if( cx < 120 )
				{
					t->m_angle = (rand() % TURN_ANGLE);
				}
				else
				{
					t->m_angle = -(rand() % TURN_ANGLE);
				}
				if( cy > 40 )
				{
					t->m_speed = 700 + (rand() & 63);
				}
				else
				{
					t->m_speed = 300;
				}
				//t->t_x = 120 + rand() % 80;
				t->t_x = 20 + rand() % (240-40);
				t->t_y = 20 + rand() % 120;
			}
			bigtankAimAtTarget( t );
			if(! t->s_missile )
			{
				t->s_missile = 30 + (rand() % 100);
				bigtankFireMissile( t );
			}
			else
			{
				t->s_missile--;
			}
			
			
			break;
			
		}
	}
	else
	{
		t->speed = (t->speed * 254) >> 8;
		if( t->y >= (160+40)<<8 )
		{
			t->flags=0;
		}
	}
	t->y += level_scroll_speed*2;
	t->y -= t->speed;// (sin_tab[((t->angle>>8)-64)&255] * t->speed) >> 14;
	t->x += (sin_tab[((t->angle>>8))&255] * t->speed) >> 14;
	
	if( t->hp )
	{
		t->cb.x = (t->x>>8) - 32;
		t->cb.y = (t->y>>8) - 32;
		t->cb.w = 64;
		t->cb.h = 64;
	}
	else
	{
		t->cb.w=0;
	}
	if( t->cb.damage )
	{
		if( t->hp - t->cb.damage <= 0 )
		{
			bulletFire( t->x>>8,t->y>>8,0,130,99,BEHAVIOR_SPEC1,255,1,3);
			t->cb.w=0;
			addPoints( t->cb.src, 53300 );
			t->hp=0;
			levelEnd();
		}
		else
		{
			t->flash = FLASHTIME;
			t->hp -= t->cb.damage;
		}
		t->cb.damage=0;
	}
}

void bigtankDraw( boss_bigtank* t )
{
	sprite* s_butt;
	sprite* s_head;
	sprite* s_turret;
	int center_x, center_y;
	center_x = t->x>>8;
	center_y = t->y>>8;
//	spriteAdd( t->t_x, t->t_y, 0, SPRITE_SIZE_8, 0 );
	int angle, anglev;
	angle = ((t->angle>>8) - 64) & 255;
	anglev = t->angle>>8;
//	t->jitter = !t->jitter;
//	if( t->jitter )
//		angle += 1;
//	else
//		angle -= 1;
	//s_head = spriteAdd( center_x + ((sin_tab[ (angle)&255 ] * 8) >> 14) - 32, center_y + ((sin_tab[ (angle+64)&255 ] * 48) >> 14) - 32, tileEntry, SPRITE_SIZE_64, 0 );
	//s_butt = spriteAdd( center_x + ((sin_tab[ (angle)&255 ] * 8) >> 14) - 32, center_y - ((sin_tab[ (angle+64)&255 ] *64) >> 14) - 32, tileEntry +64, SPRITE_SIZE_64, 0 );
	
	int t_angle = (t->t_angle - 64) & 255;
	if( t->hp )
	{
		s_turret = spriteAdd( center_x + ((sin_tab[ (t_angle+64)&255 ] * 8+8192) >> 14) -32 , center_y + ((sin_tab[ (t_angle)&255 ] * 8+8192) >> 14) -32, tileEntry +8, SPRITE_SIZE_64, 3 );
		spriteEnableRotation( s_turret, spr_affine_turret, 0 );
	
	
		spriteSetAffineMatrix( spr_affine_turret, (sin_tab[((t->t_angle)+64) & 255])>>6, (sin_tab[t->t_angle])>>6, -((sin_tab[t->t_angle])>>6), (sin_tab[((t->t_angle)+64) & 255])>>6 );
	
		s_head = spriteAdd( center_x + ((sin_tab[ (angle+64)&255 ] * 8+8192) >> 14) -64 , center_y + ((sin_tab[ (angle)&255 ] * 8+8192) >> 14) -64, tileEntry , SPRITE_SIZE_64, 3 );
		s_butt = spriteAdd( center_x - ((sin_tab[ (angle+64)&255 ] * 8+8192) >> 14) -64 , center_y - ((sin_tab[ (angle)&255 ] * 8+8192) >> 14) -64, tileEntry +64, SPRITE_SIZE_64, 3 );
		
		if( t->flash )
		{
			t->flash--;
			spriteAddPalette( s_butt, 2 );
			spriteAddPalette( s_head, 2 );
			spriteAddPalette( s_turret, 2 );
		}
	}
	else
	{
		s_head = spriteAdd( center_x + ((sin_tab[ (angle+64)&255 ] * 8+8192) >> 14) -64 , center_y + ((sin_tab[ (angle)&255 ] * 8+8192) >> 14) -64, tileEntry +16, SPRITE_SIZE_64, 3 );
		s_butt = spriteAdd( center_x - ((sin_tab[ (angle+64)&255 ] * 8+8192) >> 14) -64 , center_y - ((sin_tab[ (angle)&255 ] * 8+8192) >> 14) -64, tileEntry +64+16, SPRITE_SIZE_64, 3 );
	}
	spriteEnableRotation( s_butt, spr_affine_body, 1 );
	spriteEnableRotation( s_head, spr_affine_body, 1 );
	
	spriteSetAffineMatrix( spr_affine_body, (sin_tab[((anglev)+64) & 255])>>6, (sin_tab[anglev])>>6, -((sin_tab[anglev])>>6), (sin_tab[((anglev)+64) & 255])>>6 );
}
