#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "enemies.h"
#include "boss_tom.h"
#include "sprites.h"
#include "textlayer.h"
#include "level.h"
#include "gfx_tom.h"
#include "bullet.h"
#include "effect.h"
#include "ingame.h"
#include "tables.h"
#include "plane.h"
#include "powerup.h"
extern u8 song_messages[];

#include <maxmod.h>
#include "mmsolution.h"

#define STAGE_APPEAR 1
#define STAGE_SPEECH 2
#define STAGE_MOVE 3
#define STAGE_DISSAPEAR_MOVE 4
#define STAGE_DISSAPEAR 5
#define STAGE_ATTACK2 6
#define STAGE_APPEAR2 7
#define STAGE_DEMORALIZATION 8
#define STAGE_DEATH_POSITION 10
#define STAGE_DEATH_FLASH 11
#define STAGE_EXPLODE 12
#define STAGE_END 13
#define STAGE_DESTROYED 99

#define mstage_open1	1
#define mstage_attack1	2
#define mstage_close1	3

#define tileEntry (144*4)

#define appear_speed 120

const char tom_speech[] = "\
This is as far as you go!!";

#define HIT_POINTS 2000

#define MOVE_SPEED 200

#define tile_laser (160/8)

#define mouth_open_vulnerable 5

#define cosine(x) sin_tab[((x)+64)&255]
#define sine(x) sin_tab[(x)&255]

int shuriken_rot;
#define shuriken_affine 20
#define TS_ROT2_SPD	12

//2345678901234567890123456789
void SPAWN_TOM()
{
	LZ77UnCompVram( (void*)gfx_tomTiles, (void*)0x6014800 );
	tom_cruise* t;
	t = (tom_cruise*)malloc( sizeof( tom_cruise ) );
	memset( t, 0, sizeof( tom_cruise ) );
	
	t->cb.t = CTYPE_ENEMY;
	t->x = 120<<8;
	t->y = -48<<8;
	t->stage = STAGE_APPEAR;
	//t->stage = STAGE_DEATH_POSITION;
	//t->stage = STAGE_DISSAPEAR_MOVE;
	t->hp = HIT_POINTS;
	t->mouth = 0;
	t->flags = EFLAG_ACTIVE;
	t->timer1=0;
	t->mstage=0;
	t->mtimer=0;
	enemyRegister( (enemy*)t, ETYPE_TOM );
	mmStop();
	t->laser_active = 0;
	t->death_flash = 0;
	t->dead = 0;
	int i;
	for( i = 0; i < 15; i++ )
		song_messages[i]=0;
	
}

void tomExplode( tom_cruise* t )
{
	t->stage = STAGE_EXPLODE;
	
	int cx,cy;
	cx = t->x>>8;
	cy = t->y>>8;
	
	cx -= 32;
	cy -= 45;
	
	// make pieces
	int x, y;
	for( x = 0; x < 8; x++ )
	{
		for( y = 0; y < 11; y++ )
		{
			t->pieces[x+(y<<3)].x = (cx + x*8)<<6;
			t->pieces[x+(y<<3)].y = (cy + y*8)<<6;
			t->pieces[x+(y<<3)].vx = (x - 4) * 20 + 10+ (rand() % 70) - 30;
			t->pieces[x+(y<<3)].vy = (y - 5) * 20 - 50+ (rand() % 70) - 30;
			t->pieces[x+(y<<3)].rot = 0;
			t->pieces[x+(y<<3)].vr = (rand() & 31) - 15;
			if( t->pieces[x+(y<<3)].vr == 0 )
				t->pieces[x+(y<<3)].vr = 5;
			t->pieces[x+(y<<3)].z = 256;
			t->pieces[x+(y<<3)].vz = 2 + rand() % 5;
		}
	}
	textClear( TEXT_START_X, TEXT_START_Y+5, 32 );
	textDraw( TEXT_START_X, TEXT_START_Y+5, 50, "NOOOOOOOOOOOOOOOOOOO!!!" );
	bulletFire( t->x>>8,t->y>>8,0,0,99,BEHAVIOR_SPEC1,255,1,3);
	p1.score = 0x99999999;
	stats_dirty_p1 = 1;
}

int tomGotoMiddle( tom_cruise* t )
{
	int p = 1;
	if( (t->x) < (120<<8) )
	{
		t->x += 256;
		p =0 ;
	}
	else if( (t->x) >= (121<<8) )
	{
		t->x -= 256;
		p = 0;
	}
	if( (t->y) < (80<<8) )
	{
		t->y += 256;
		p=0;
	}
	else if( (t->y) >= (81<<8) )
	{
		t->y -= 256;
		p=0;
	}
	return p;
}

void tomUpdate( tom_cruise* t )
{
	switch( t->stage )
	{
	case STAGE_APPEAR:
		t->y += appear_speed;
		if( t->y >= (80<<8) )
		{
			textDraw( TEXT_START_X, TEXT_START_Y+5, TEXT_SPEED,(char*) tom_speech );
			t->stage = STAGE_SPEECH;
		}
		break;
	case STAGE_SPEECH:
	case STAGE_DEMORALIZATION:
		if( text_write_active )
		{
			if( t->timer1 )
			{
				t->timer1--;
			}
			else
			{
				if( !t->mouth )
				{
					t->mouth = 1 + (rand()%3);
				}
				else
				{
					t->mouth = 0;
				}
				t->timer1 = rand()%7;
			}
		}
		else
		{
			t->mouth=0;
			t->timer1++;
			if( t->timer1 >= 150 )
			{
				if( t->stage == STAGE_SPEECH )
					mmStart( MOD_BOSS3, MM_PLAY_LOOP );
				textClear( TEXT_START_X, TEXT_START_Y+5, 64 );
				t->timer1=0;
				t->stage = STAGE_MOVE;
			}
		}
		break;
	case STAGE_MOVE:
		if( t->bounce_x )
		{
			t->x += MOVE_SPEED;
			if( t->x >= (240-8)<<8 )
				t->bounce_x = 0;
		}
		else
		{
			t->x -= MOVE_SPEED;
			if( t->x < (8)<<8 )
				t->bounce_x = 1;
		}
		if( t->bounce_y )
		{
			t->y += MOVE_SPEED;
			if( t->y >= (160-8-45)<<8 )
				t->bounce_y = 0;
		}
		
		else
		{
			t->y -= MOVE_SPEED;
			if( t->y < (10)<<8 )
				t->bounce_y = 1;
		}
		if( t->mstage == 0 )
		{
			t->timer1++;
			if( t->timer1 >= 100 )
			{
				t->mstage = mstage_open1;
				t->timer1=0;
			}
		}
		if( song_messages[2] )
		{
			song_messages[2] = 0;
			song_messages[3]=0;
			
			if( t->mstage == mstage_attack1 )
			{
				t->mstage = mstage_close1;
			}
			else
			{
				t->mouth =0 ;
				t->laser_active=0;
				t->mstage=0;
			}
			t->mtimer=0;
			t->stage = STAGE_DISSAPEAR_MOVE;
			
		}
		break;
	case STAGE_DISSAPEAR_MOVE:
		if( tomGotoMiddle( t ) )
		{
			t->death_flash++;
			if( song_messages[3] )
			{
				song_messages[3]=0;
				t->death_flash=0;
				t->timer1=0;
				t->stage = STAGE_DISSAPEAR;
				t->disp_scale = 65536;
				t->disp_t = 0;
				t->disp_f = 0;
				flashScreen( 12 );
				textDraw( TEXT_START_X, TEXT_START_Y+5, TEXT_SPEED,(char*)"HA! HA! HA!" );
				t->timer2=0;
				powerupSpawn( t->x>>8, t->y>>8, POWERUP_1UP );
			}
		}
		break;
	case STAGE_DISSAPEAR:
		song_messages[3]=0;
		song_messages[1]=0;
		song_messages[8]=0;
		song_messages[9]=0;
		t->disp_t++;
		t->disp_scale = (t->disp_scale * 220) >> 8;
		t->y -= 1000;
		if( t->y < -45<<8 )
		{
			t->y = -45<<8;
			t->stage = STAGE_ATTACK2;
			song_messages[8]=0;
			
		}
		if( t->disp_scale < 1000 )
		{
			t->disp_scale = 1000;
			t->disp_f++;
			
		}
		break;
	case STAGE_ATTACK2:
		if( t->timer2 < 100 )
		{
			t->timer2++;
			if( t->timer2 >= 100 )
			{
				textClear( TEXT_START_X, TEXT_START_Y+5, 32 );
			}
		}
		if( level_scroll_speed < 1400 )
		{
			level_scroll_speed = ((level_scroll_speed * 260) >> 8) + 1;
			song_messages[8]=0;
		}
		else
		{
			if( t->timer1 <= 0 )
			{
				t->timer1 = 2 + rand() % 5;
				tomlaserSpawn( 5 + rand() % 240 - 5 - 5 );
			}
			else
			{
				t->timer1--;
			}
			if( song_messages[8] )
			{
				song_messages[8]=0;
				tommystarSpawn( 240+200, 160, 0 );
				tommystarSpawn( 240+200, 0, 0 );
				tommystarSpawn( -200, 80, 1 );
			}
			if( song_messages[1] )
			{
				song_messages[1] = 0;
				t->stage = STAGE_APPEAR2;
				t->y = -45<<8;
			}
		}
		break;
	case STAGE_APPEAR2:
		if( level_scroll_speed > 80 )
		{
			level_scroll_speed = ((level_scroll_speed * 250) >> 8);
			if( level_scroll_speed < 80 )
			{
				level_scroll_speed = 80;
			}
		}
		else
		{
			if( tomGotoMiddle( t ) && level_scroll_speed == 80 )
			{
				t->stage = STAGE_DEMORALIZATION;
				t->timer1=0;
				textDraw( TEXT_START_X, TEXT_START_Y+5, TEXT_SPEED,(char*)"You don't stand a chance\ragainst me!" );
			}
		}
		
		break;
	case STAGE_DEATH_POSITION:
		{
			
			if( tomGotoMiddle( t ) )
			{
				t->stage = STAGE_DEATH_FLASH;
				t->timer1=0;
				textDraw( TEXT_START_X, TEXT_START_Y+5, TEXT_SPEED,(char*)"Don't think you have won." );
			}
			t->death_flash++;
		}
		break;
	case STAGE_DEATH_FLASH:
		if( text_write_active )
		{
			if( t->timer1 )
			{
				t->timer1--;
			}
			else
			{
				if( !t->mouth )
				{
					t->mouth = 1 + (rand()%3);
				}
				else
				{
					t->mouth = 0;
				}
				t->timer1 = rand()%7;
			}
		}
		else
		{
			t->mouth=0;
			t->timer1++;
			if( t->timer1 >= 60 )
			{
				tomExplode( t );
			}
		}
		t->death_flash++;
		break;
	case STAGE_EXPLODE:
		{
			t->timer1++;
			if( t->timer1 < 200 )
			{
				if(( t->timer1 & 3 )== 0 )
				{
					if( rand() % 2 )
					{
						effectStart( (t->x>>8)-30 + (rand() % 60), (t->y>>8)-30 + (rand() % 60), 0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0 );
						
					}
				}
			}
			else if( t->timer1==200)
			{
				levelEnd();
			}
			t->timer2++;
			if( t->timer1 < 100 )
			{
				if( t->timer2 >= 2 )
				{
					t->timer2 = 0;
					flashScreen ( 7 );
				}
			}
			else if( t->timer1 < 150 )
			{
				if( t->timer2 >= 3 )
				{
					t->timer2 = 0;
					flashScreen ( 6 );
				}
			}
			else if( t->timer1 < 200 )
			{
				if( t->timer2 >= 5 )
				{
					t->timer2 = 0;
					flashScreen ( 6 );
				}
			}
			else if( t->timer1 < 240 )
			{
				if( t->timer2 >= 5 )
				{
					t->timer2 = 0;
					flashScreen ( 3 );
				}
			}
			int p;
			int updated = 0;
			#define cp t->pieces[p]
			for( p = 0; p < 88; p++ )
			{	
				if( (cp.y>>6) < 170 )
				{
					updated = 1;
					cp.x += cp.vx;
					cp.y += cp.vy;
					cp.z -= cp.vz;
					cp.rot += cp.vr;
				//	if( cp.z < 0 )
				//		cp.z = 0;
					if( cp.vy < 500 )
						cp.vy++;
					
				}
			}
			
			#undef cp
			if( !updated )
			{
				textClear( TEXT_START_X, TEXT_START_Y+5, 32 );
				t->stage = STAGE_END;
				t->timer1=0;
			}
		}
		break;
	case STAGE_END:
		t->timer1++;
		if( t->timer1 >= 1 )
		{
			
			t->stage = STAGE_DESTROYED;
			t->flags = 0;
		}
	}
	switch( t->mstage )
	{
	case mstage_open1:
		t->mouth++;
		if( t->mouth >= 9 )
			t->mstage = mstage_attack1;
		break;
	case mstage_attack1:
		t->mtimer++;
		if( t->mtimer >= 60+20*3 || t->dead )
		{
			t->mstage = mstage_close1;
			t->mtimer=0;
		}
		else
		{
			
			if( !t->laser_active )
			{
				if( (t->mtimer) >= 20 )
				{
					t->laser_active=1;
					t->laser_length=0;
				}
			}
			else
			{
				t->laser_length += 20;
				if( t->laser_length > 200 ) 
					t->laser_length=200;
				t->laser_flash++;
				t->laser_flash %= 4;
				if( t->laser_flash == 0 )
				{
					mmEffectCancel( t->sfx_laser );
					t->sfx_laser = mmEffect( SFX_LASER );
					mmEffectFrequency( t->sfx_laser, 4000 );
					mmEffectVolume( t->sfx_laser, 100 );
				}
			}
			/*if(!  )
			{
				t->laser_active=1;
				t->laser_length=0;
			//	bulletFire( (t->x>>8)-3, (t->y>>8)+20, t->bounce_x?MOVE_SPEED>>1:-MOVE_SPEED>>1, 900, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_p3 );
			//	bulletFire( (t->x>>8)+3, (t->y>>8)+20, t->bounce_x?MOVE_SPEED>>1:-MOVE_SPEED>>1, 900, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_p3 );
		//		bulletFire( t->x>>8, (t->y>>8)+20, -80, 141, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_large );
			//	bulletFire( t->x>>8, (t->y>>8)+20, 80, 141, CTYPE_PLAYER, BEHAVIOR_STATIC, 0, 1, bullet_tile_large );
			}*/
		}
		break;
	case mstage_close1:
		t->laser_active=0;
		t->mouth --;
		if( t->mouth <= 0 )
		{
			t->mouth=0;
			t->mstage=0;
		}
	}
	
//	if( t->mouth >= mouth_open_vulnerable )
//	{
		t->cb.x = (t->x>>8) - 32 + 19;
		t->cb.y = (t->y>>8) - 45 + 70-4;
		t->cb.w = 23;
		t->cb.h= 10+4;
//	}
//	else
//	{
//		t->cb.w = 0;
//	}
	if( !t->dead )
	{
	
		if( t->cb.damage )
		{
			if( t->mouth >= mouth_open_vulnerable )
			{
				t->hp -= t->cb.damage;
				t->flash = 2;
			}
			t->cb.damage=0;
			if( t->hp <= 0 )
			{
				t->dead=1;
				t->stage = STAGE_DEATH_POSITION;
		//		t->laser_active=0;
		//		t->mouth=0;
		//		t->laser_length=0;
		//		t->laser_flash=0;
				mmStop();
			}
		}
	
		if( t->laser_active )
		{
			int ay;
			collbox* cb;
			for( ay =0 ;ay < t->laser_length; ay += 16 )
			{
				cb = collisionCheck( (t->x>>8)-32 + 30+4, (t->y>>8)-45+70+ay, CTYPE_PLAYER );
				if( cb )
					collisionHit( cb, 1, CSRC_ENEMY );
				cb = collisionCheck( (t->x>>8)-32 + 30 -4, (t->y>>8)-45+70+ay, CTYPE_PLAYER );
				if( cb )
					collisionHit( cb, 1, CSRC_ENEMY );
			}
		}
	
		{
			int tx,ty;
			tx =  (t->x>>8)-32;
			ty = (t->y>>8)-45;
			collbox* cb;
			cb = collisionFind( (t->x>>8)-32 + 8, (t->y>>8)-45+8, (t->x>>8)-32 + 55, (t->y>>8)-45+71, CTYPE_PLAYER );
			if( cb )
				collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+21, ty+1, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+32, ty+0, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			
			cb = collisionCheck( tx+11, ty+75, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+17, ty+81, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+21, ty+1, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+23, ty+85, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+31, ty+85, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+39, ty+84, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+47, ty+78, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
			cb = collisionCheck( tx+51, ty+73, CTYPE_PLAYER );
			if( cb ) collisionHit( cb, 1, CSRC_ENEMY );
		}
	}
	shuriken_rot += TS_ROT2_SPD;
	shuriken_rot &= 255;
	spriteSetAffineMatrix( shuriken_affine, (sin_tab[((shuriken_rot)+64) & 255])>>6, (sin_tab[shuriken_rot])>>6, -((sin_tab[shuriken_rot])>>6), (sin_tab[((shuriken_rot)+64) & 255])>>6 );
}

void tomDraw( tom_cruise* t )
{
	if( t->stage <= STAGE_DEATH_FLASH )
	{
		if( t->stage != STAGE_DISSAPEAR )
		{
			sprite *top,*bottom,*mouth, *mouthback;
			sprite *mouthfix;
			mouth = spriteAdd( (t->x>>8) -32 + 19, (t->y>>8) - 45 + 70 + t->mouth, tileEntry+8, SPRITE_SIZE_32, 2 );
			mouthfix = spriteAdd( (t->x>>8) -32 +16, (t->y>>8) - 45 + 64, tileEntry+16, SPRITE_SIZE_32, 1 );
			mouthback = spriteAdd( (t->x>>8) - 32 + 19, (t->y>>8) - 45 + 70, tileEntry+12, SPRITE_SIZE_32, 3 );
			top = spriteAdd( (t->x>>8) -32, (t->y>>8) -45, tileEntry, SPRITE_SIZE_64, 1 );
			bottom = spriteAdd( (t->x>>8) - 32, (t->y>>8) - 45 + 64, tileEntry +64*4, SPRITE_SIZE_64, 3 );
			spriteWide( bottom );
			
			if( t->laser_active && t->laser_flash != 0 )
			{
				int y;
				int l;
				for( y = (t->y>>8) - 45 + 70, l=0; (y < 160) && (l < t->laser_length); y+= 32, l+= 32 )
				{
					spriteTall( spriteAdd((t->x>>8) -32 + 22, y, tileEntry+tile_laser, SPRITE_SIZE_32, 0 ) );
				}
			}
			int p=0;
			if( (t->death_flash & 3) >= 3)
				p = 1;
			if( t->flash )
			{
				p = 2;
				t->flash--;
			}
			if( p )
			{
				spriteAddPalette( top, p );
				spriteAddPalette( bottom, p  );
				spriteAddPalette( mouth, p );
				spriteAddPalette( mouthfix, p );
			}
		}
		else
		{
			sprite* top, *bottom;
			top = spriteAdd( (t->x>>8) -32, (t->y>>8) -45, tileEntry, SPRITE_SIZE_64, 1 );
			bottom = spriteAdd( (t->x>>8) - 32, (t->y>>8) - 45 + 64, tileEntry +64*4, SPRITE_SIZE_64, 3 );
			spriteWide( bottom );
			spriteEnableRotation( top, 19, 0 );
			spriteEnableRotation( bottom, 19, 0 );
			spriteSetAffineMatrix( 19, (65536<<8)/t->disp_scale, 0, 0, 256 );
			spriteAddPalette( top, 3 );
			spriteAddPalette( bottom, 3 );
		}
	}
	else
	{
		int p;
		#define cp t->pieces[p]
		for( p = 0; p < 88; p++ )
		{
			if( (cp.y>>6) < 170 && (cp.y>>6) >= -8 )
			{
				//if(( rand()  % (cp.z)) > 2 )
				//{
					sprite* s = spriteAdd( (cp.x>>6) -4, (cp.y>>6)-4, tileEntry + ((p & 7) + ((p>>3)*32)), SPRITE_SIZE_8, 2 );
					spriteEnableRotation( s, ((cp.rot+8)>>4) & 15, 1 );
				//}
			}
		}
		#undef cp
	}
	
}

//---------------------------------------------------------------------------------------

#define TL_SPEED 1400
#define TL_TILE ((208/8) + (144*4))

void tomlaserSpawn( int x )
{
	tomlaser* t;
	t = (tomlaser*)malloc( sizeof( tomlaser ) );
	t->x = x<<6;
	t->y = -100<<6;
	t->flags = EFLAG_ACTIVE;
	enemyRegister( (enemy*)t, ETYPE_TOMLASER );
}

void tomlaserUpdate( tomlaser* t )
{
	if( t->y < (200<< 6) )
	{
		t->y += TL_SPEED;
//		collbox* cb;
		/*cb = collisionCheck( (t->x>>6), (t->y>>6), CTYPE_PLAYER );
		if( cb )
			collisionHit( cb, 1, CSRC_ENEMY );
		cb = collisionCheck( (t->x>>6), (t->y>>6)+56, CTYPE_PLAYER );
		if( cb )
			collisionHit( cb, 1, CSRC_ENEMY );*/
	}
	else
	{
		t->flags=0;
	}
}

void tomlaserDraw( tomlaser* t )
{
	sprite* s ;
	
	/*if( (t->y>>6)+40  < 160 )
	{
		s = spriteAdd( (t->x>>6)-5, (t->y>>6) + 40,  TL_TILE, SPRITE_SIZE_64, 2 );
		spriteTall( s );
	}*/
	if( (t->y>>6)  < 160 )
	{
		s = spriteAdd( (t->x>>6)-5, t->y>>6,  TL_TILE, SPRITE_SIZE_64, 2 );
		spriteTall( s );
	}
}

//--------------------------------------------------------------------------------------
#define TS_SPEED 500
#define TS_TILE ((144*4)+(64/8)+(32*4))

#define TS_ROT_SPD	-192

#define ts_distance 40

void tommystarSpawn( int x, int y, int dir )
{
	tommystar* t;
	t = (tommystar*)malloc( sizeof( tommystar ) );
	t->x = x<<8;
	t->y = y<<8;
	t->dir = dir;
	t->flags = EFLAG_ACTIVE;
	t->rot = rand();
	enemyRegister( (enemy*)t, ETYPE_TOMMYSTAR );
}

void tommystarUpdate( tommystar* t )
{
	if( !t->dir )
	{
		t->x -= TS_SPEED;
		if( t->x <= (-ts_distance - 32)<<8 )
			t->flags=0;
	}
	else if( t->dir == 1 )
	{
		t->x += TS_SPEED;
		if( t->x >= (240+ts_distance + 32)<<8 )
			t->flags=0;
	}
	int cx,cy;
	cx = (t->x>>8) ;
	cy = (t->y>>8 );
	
//	collbox* cb;
	int x,y,s;
	for( s = 0; s < 3; s++ )
	{
		x = cx + ((cosine(((t->rot>>8) + (s*85)))*ts_distance)>>14);
		y = cy +((sine(((t->rot>>8)+(s*85)))*ts_distance)>>14);
		/*cb = collisionCheck( x, y+5, CTYPE_PLAYER );
		if( cb )
			collisionHit( cb, 1, CSRC_ENEMY );*/
		//cb = collisionFind( x-7, y-7, x+7,y+7, CTYPE_PLAYER );
		
		if( p1.cb.w )
		{
			int x2 = (p1.x >>8) + 11;
			int y2 = (p1.y >>8) + 14;
			x2 -= x;
			y2 -= y;
			x2 = x2 * x2;
			y2 = y2 * y2;
			if( x2+y2 < (14*14) )
			//if( x2 >= x-13 && x2 < x+13 && y2 >= y-13 && y2 < y+13 )
			{
				collisionHit( &p1.cb, 1, CSRC_ENEMY );
			}
		}
		if( p2.cb.w )
		{
			int x2 = (p2.x >>8) + 11;
			int y2 = (p2.y >>8) + 12;
			x2 -= x;
			y2 -= y;
			x2 = x2 * x2;
			y2 = y2 * y2;
			if( x2+y2 < (14*14) )
			{
				collisionHit( &p2.cb, 1, CSRC_ENEMY );
			}
		}
		/*if( cb )
			collisionHit( cb, 1, CSRC_ENEMY );
		cb = collisionCheck( x+5, y, CTYPE_PLAYER );
		if( cb )
			collisionHit( cb, 1, CSRC_ENEMY );
		cb = collisionCheck( x, y-5, CTYPE_PLAYER );
		if( cb )
			collisionHit( cb, 1, CSRC_ENEMY );*/
	}
	t->rot += TS_ROT_SPD;
}

void tommystarDraw( tommystar* t )
{
	int s;
	int cx,cy;
	cx = (t->x>>8) - 32;
	cy = (t->y>>8 )- 32;
	
	int x,y;
	for( s = 0; s < 3; s++ )
	{
		x = cx + ((cosine(((t->rot>>8) + (s*85)))*ts_distance)>>14);
		y = cy +((sine(((t->rot>>8)+(s*85)))*ts_distance)>>14);
		if( x >= -60 && y >= -60 && x < 240+60 && y < 160+60 )
		{
			sprite* spr = spriteAdd( x, y, TS_TILE, SPRITE_SIZE_32, 2 );
			spriteEnableRotation( spr, shuriken_affine, 1 );
		}
	}
}
