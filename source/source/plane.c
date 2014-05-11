// plane code :)

#include <gba.h>
#include "sprites.h"
#include "collision.h"
#include "bullet.h"
#include "effect.h"
#include "plane.h"
#include "sections.h"
#include "comms.h"
#include "multiboot.h"
#include "powerup.h"
#include "ingame.h"
#include "level.h"

#include "maxmod.h"
#include "mmsolution.h" 

#define PLANE_START_POSX	((240/2)-8)
#define PLANE_START_POSY	(160)
#define PLANE_START_ALT		90

#define cnt_disable	0
#define cnt_appear	1
#define cnt_user	2
#define cnt_slave	3

#define boundary_top		(2<<8)
#define boundary_bottom		((160-24-2)<<8)
#define boundary_left		(2<<8)
#define boundary_right		((240-24-2)<<8)

#define plane_horz_speed	200
#define plane_horz_maxvel	450

#define plane_vert_speed	200
#define plane_vert_maxvel	450

#define tile_plane_fire	128
#define tile_plane	0

#define plane_reload_time 7

#define shadow_tweak_x	20
#define shadow_tweak_y	24

plane p1;// GAMEDATA;
plane p2;// GAMEDATA;

void planeSetup( void )
{
	p1.cb.t = CTYPE_PLAYER;
	p2.cb.t = CTYPE_PLAYER;
	collisionRegister( &p1.cb );
	collisionRegister( &p2.cb );
}

void planeReset( plane* p, int x, int slave )
{
	p->x 		= (x-16)<<8;
	p->y 		= PLANE_START_POSY<<8;
	p->a 		= PLANE_START_ALT;
	p->cnt 		= cnt_appear;
	p->vx		= 0;
	p->vy		= -300;
	p->hs		= 0;
	p->vs		= 0;
	p->reload	= 0;
	p->active	= 1;
	p->missiles	= 2;
	p->invun 	= 180; 
	p->power	= 0;
	p->cb.damage=0;
	if( slave )
		p->cnt = cnt_slave;
}

void planeLevel( plane* p, int speed )
{
	if( p->a < 256 )
	{
		p->a += speed;
		if( p->a > 256 ) p->a = 256;
	}
	else if( p->a > 256 )
	{
		p->a -= speed;
		if( p->a < 256 ) p->a = 256;
	}
}

void planeSteerLeft( plane* p )
{
	p->hs = 1;
}

void planeSteerRight( plane* p )
{
	p->hs = 2;
}

void planeAccel( plane* p )
{
	p->vs = 1;
}

void planeSlow( plane* p )
{
	p->vs = 2;
}

void planeShoot( plane* p )
{
	if( p->firing == 0 )
	{
		p->firing=4;
	}
}
 
void planeShootInternal( plane* p )
{
	int damage;
	mm_sfxhand s;
	int pu = p == &p1 ? CSRC_P1 : CSRC_P2;
	//p->shot_count++;
	switch( p->power )
	{
	case 0:
		damage = 5;
		bulletFireP( (p->x>>8) + 7, (p->y>>8)+8, 0, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p1, pu );
		bulletFireP( (p->x>>8) + 16, (p->y>>8)+8, 0, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p1, pu );
		mmEffectPanning( mmEffect( SFX_LASER ), (p->x>>8)+16);
		break;
	case 1:
		damage = 9;
		s = mmEffect( SFX_LASER );
		bulletFireP( (p->x>>8) + 7, (p->y>>8)+8, 0, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p2, pu );
		bulletFireP( (p->x>>8) + 16, (p->y>>8)+8, 0, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p2, pu );
		mmEffectPanning( s, (p->x>>8)+16);
		mmEffectFrequency( s, 11000 );
		break;
	case 2:
		damage = 10;
		s = mmEffect( SFX_LASER );
		bulletFireP( (p->x>>8) + 7, (p->y>>8)+8, -200, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p2, pu );
		bulletFireP( (p->x>>8) + 11, (p->y>>8)+8, 0, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p2, pu );
		bulletFireP( (p->x>>8) + 16, (p->y>>8)+8, 200, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p2, pu );
		mmEffectPanning( s, (p->x>>8)+16);
		mmEffectFrequency( s, 10000 );
		break;
	case 3:
		damage = 9;
		s = mmEffect( SFX_LASER );
		bulletFireP( (p->x>>8) + 7, (p->y>>8)+8, -200, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p3, pu );
		bulletFireP( (p->x>>8) + 11, (p->y>>8)+8, 0, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p3, pu );
		bulletFireP( (p->x>>8) + 16, (p->y>>8)+8, 200, -1000, CTYPE_ENEMY, BEHAVIOR_STATIC, 0, damage, bullet_tile_p3, pu );
		if( p->firing==3 )
			planeFireMissileInternal( p );
		mmEffectPanning( s, (p->x>>8)+16);
		mmEffectFrequency( s, 8000 );
		break;
	}
}

void planeFireMissile( plane* p )
{
	if( p->firing_m == 0 )
	{
		p->firing_m=1;
	}
}

void planeFireMissileInternal( plane* p )
{	
	int damage;
	damage = 20;
	mm_sfxhand s = mmEffect( SFX_MISSILE );
	mmEffectPanning(s , (p->x>>8)+16);
	mmEffectVolume( s, 200 );
	//mmEffectFrequency( s, 10000 );
	int mx;
	p->mslot = !p->mslot;
	mx = p->mslot ? 6 : -6;
	int pu = p == &p1 ? CSRC_P1 : CSRC_P2;
	bulletFireP( (p->x>>8)+ 12+mx, (p->y>>8)+12, 0, -200, CTYPE_ENEMY, BEHAVIOR_SMART, 0, damage, bullet_tile_missile, pu );
}

void planePowerup( plane* p )
{
	int a = p->power;
	a++;
	if( a > POWER_MAX ) a = POWER_MAX;
	p->power = a;
	p->power_time = 60*20;
}

void planeDropBomb( plane* p )
{
	if( !p->firing_b && !p->reload_b )
	{
		if( p->invun <= 30 )
		{
			p->firing_b = 1;
		}
		//smissileSpawn( p->x>>8, p->y>>8 );
		//dmissileSpawn( p->x>>8, p->y>>8, 0 );
		//invaderSpawn( p->x>>8, p->y>>8 );
	}
}

void planeDropBombInternal( plane* p )
{
	int damage;
	damage = 40;
	mm_sfxhand s = mmEffect( SFX_MISSILE );
	mmEffectPanning(s , (p->x>>8)+16);
	mmEffectVolume( s, 200 );
	mmEffectFrequency( s, 8000 );
	int mx;
	p->mslot = !p->mslot;
	mx = p->mslot ? 6 : -6;
	int pu = p == &p1 ? CSRC_P1 : CSRC_P2;
	bulletFireP( (p->x>>8)+ 12+mx, (p->y>>8)+12, 0, -20, CTYPE_ENEMY, BEHAVIOR_SPEC1, 0, damage, bullet_tile_bomb, pu );
	effectStart( (p->x>>8)+ 12+mx, (p->y>>8)+12,0,40,effect_smoke_start,4,0,effect_smoke_len,0,0);
	effectStart( (p->x>>8)+ 12+mx, (p->y>>8)+12,-20,40,effect_smoke_start,4,0,effect_smoke_len,0,0);
	effectStart( (p->x>>8)+ 12+mx, (p->y>>8)+12,20,40,effect_smoke_start,4,0,effect_smoke_len,0,0);
	p->invun = 170;
}

void planeUpdate( plane* p )
{
	if( p->active )
	{
		switch( p->cnt )
		{
		case cnt_appear:
			p->y += p->vy;
			if( p->vy < -30)
			{
				if( p->y < (128<<8) )
					p->vy = (p->vy * 240) >> 8;
			}
			else
			{
				p->cnt = cnt_user;
				p->vy = 0;
				planeLevel( p, 100 );
			}
			p->firing=0;
			p->firing_b=0;
			planeLevel( p, 3 );
			break;
		case cnt_user:
			p->y += p->vy;
			if( p->y < boundary_top ) p->y = boundary_top;
			if( p->y > boundary_bottom ) p->y = boundary_bottom;
			p->x += p->vx;
			if( p->x < boundary_left ) p->x = boundary_left;
			if( p->x > boundary_right ) p->x = boundary_right;
			if( p->hs == 1 )
			{
				p->hs=0;
				if( p->vx > -(plane_horz_maxvel) ) p->vx -= plane_horz_speed;
				if( p->vx < -(plane_horz_maxvel) ) p->vx = -plane_horz_maxvel;
			}
			else if( p->hs == 2 )
			{
				p->hs=0;
				if( p->vx < (plane_horz_maxvel) ) p->vx += plane_horz_speed;
				if( p->vx > (plane_horz_maxvel) ) p->vx = plane_horz_maxvel;
			}
			else
			{
				if( p->vx > plane_horz_speed )
				{
					p->vx -= plane_horz_speed;
				}
				else if( p->vx < -plane_horz_speed )
				{
					p->vx += plane_horz_speed;
				}
				else
					p->vx=0;
			}
			
			if( p->vs == 1 )
			{
				p->vs=0;
				if( p->vy > -(plane_vert_maxvel) ) p->vy -= plane_vert_speed;
				if( p->vy < -(plane_vert_maxvel) ) p->vy = -plane_vert_maxvel;
			}
			else if( p->vs == 2 )
			{
				p->vs=0;
				if( p->vy < (plane_vert_maxvel) ) p->vy += plane_vert_speed;
				if( p->vy > (plane_vert_maxvel) ) p->vy = plane_vert_maxvel;
			}
			else
			{
				if( p->vy > plane_vert_speed )
				{
					p->vy -= plane_vert_speed;
				}
				else if( p->vy < -plane_vert_speed )
				{
					p->vy += plane_vert_speed;
				}
				else
					p->vy=0;
			}
			if( p->reload == 0 )
			{
				if( p->firing )
				{
					p->firing--;
					p->reload = plane_reload_time;
					planeShootInternal( p );
					
					p->shot=1;
				}
			}
			else
			{
				p->reload--;
			}
			
			if( p->firing_b )
			{
				p->firing_b =0;
				if( p->missiles )
				{
					p->missiles--;
					
					if( p == &p1 )
						stats_dirty_p1 = 1;
					else
						stats_dirty_p2 = 1;
					
					planeDropBombInternal( p );
					p->b_shot= 1;
				}
			}
			else
			{
				if( p->reload_b )
					p->reload_b--;
			}
			break;
		case cnt_slave:
			break;
		}
		p->blastfire++;
		if( p->invun && (p->cnt != cnt_slave) )
		{
			p->invun--;
		}
		if( p->power_time )
		{
			p->power_time--;
			if( !p->power_time )
			{
				if( p->power > 1 )
				{
					p->power_time = 60*20;
					p->power--;
				}
				
			}
		}
		
		p->cb.w =12;
		p->cb.h =14;
		p->cb.x =(p->x>>8)+6;
		p->cb.y =(p->y>>8)+6;
		if( p->reload != 0 )
			p->reload--;
			
		if( p->cb.damage )
		{
			if( !p->invun )
			{
				effectStart( (p->x>>8) + 12, (p->y>>8) + 12 ,0,-level_scroll_speed,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,5);
				effectStart( (p->x>>8) + 12, (p->y>>8) + 12 ,0,-level_scroll_speed,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,-2);
				mm_sfxhand s = effectMediumExplosionS( (p->x>>8) + 12, (p->y>>8) + 12 );
				mmEffectFrequency( s, 8000 );
				p->active=0;
				p->cb.w=0;
				powerupSpawn( (p->x>>8) + 12, (p->y>>8) + 12, POWERUP_POWER );
				flashScreen( 10 );
			}
			else
			{
				p->cb.damage=0;
			}
			
		}
	}
	else
	{
		p->cb.w=0;
		p->cb.h=0;
	}
}

void planeDraw( plane* p, int affindex, int palette )
{
	if( p->active )
	{
		sprite* s;
		int px,py;
		px = (p->x>>8);
		py = (p->y>>8);
		int tile;
		if( p->vx == 0 )
			tile = 0;
		else
			tile = 4;
	
		if( !(p->invun & 1) )
		{
			int affxy;
			affxy = 65536/p->a;
			if( p->vx < 0 )
			{
				s = spriteAdd( px-10, py, tile_plane+tile, SPRITE_SIZE_32, 1 );
				spriteSetAffineMatrix( affindex,-affxy,0,0,affxy );
			}
			else
			{
				s = spriteAdd( px, py, tile_plane+tile, SPRITE_SIZE_32, 1 );
				spriteSetAffineMatrix( affindex,affxy,0,0,affxy );
			}
			if( (p->blastfire & 3) > 1 && (p->cnt == cnt_user))
			{
				spriteAdd( px+8,py+16,tile_plane_fire+(tile>>2), SPRITE_SIZE_8, 1 );
			}
			spriteEnableRotation( s, affindex, 0 );
			spriteAddPalette( s, palette );
		}
//		p->shadow_flicker ++;
//		p->shadow_flicker &= 1;
//		if( p->shadow_flicker == 0 )
//			spriteAdd( px + shadow_tweak_x, py + shadow_tweak_y, tile_plane+8, SPRITE_SIZE_8, 3 );
	}
}

void planeMakeData( plane* p, playerdata* pd )
{
	pd->flags = 0;
	if( p->shot )
	{
		pd->flags |= PDFLAG_FIRING;
		p->shot=0;
	}
	if( p->mshot )
	{
		pd->flags |= PDFLAG_FIRINGM;
		p->mshot=0;
	}
	pd->ammo = p->missiles;// + (p->nbombs<<4);
	if( p->invun )
		pd->flags |= PDFLAG_INVUN;
	if( p->vx < 0 )
		pd->flags |= PDFLAG_HFLIP;
	if( p->vx )
		pd->flags |= PDFLAG_TURNING;
	pd->x = p->x>>8;
	pd->y = p->y>>8;
	pd->a = p->a>>1;
	u16 checksum=0;
	int x;
	for( x = 0; x < (sizeof( playerdata )/2)-1; x++ )
	{
		checksum += ((u16*)pd)[x];
	}
	pd->checksum = checksum;
}

void planeReadData( plane* p, playerdata* pd )
{
	u16 checksum=0;
	int x;
	for( x = 0; x < (sizeof( playerdata )/2)-1; x++ )
	{
		checksum += ((u16*)pd)[x];
	}
	if( checksum != pd->checksum )
		return;
	if( pd->flags & PDFLAG_FIRING )
		planeShootInternal( p );
	if( pd->flags & PDFLAG_FIRINGM )
		planeFireMissileInternal( p );
	if( pd->flags & PDFLAG_INVUN )
		p->invun ^= 1;
	else
		p->invun=0;
	p->missiles = pd->ammo & 0xF;
	//p->nbombs = pd->ammo >> 4;
	if( pd->flags & PDFLAG_HFLIP )
		p->vx = -1;
	else if( pd->flags & PDFLAG_TURNING )
		p->vx=1;
	else
		p->vx=0;
	p->x = pd->x<<8;
	p->y = pd->y<<8;
	p->a = pd->a<<1;
}
