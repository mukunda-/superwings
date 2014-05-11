#include <gba.h>
#include <stdlib.h>
#include "collision.h"
#include "enemies.h"
#include "bigplane.h"
#include "heli.h"
#include "lilplane.h"
#include "tank.h"
#include "powerup.h"
#include "structure.h"
#include "boss_prototype.h"
#include "smissile.h"
#include "boss_bigtank.h"
#include "boss_tom.h"
#include "dmissile.h"
#include "invader.h"

enemy* enemy_first;
enemy* enemy_last;

void enemiesInit( void )
{
	enemy_first = 0;
	enemy_last = 0;
}

void enemiesCleanup( void )
{
	enemy* e;
//	enemy* next;
	enemy* f;
	for( e = enemy_first; e; )
	{
		f = e;
		switch( e->type )
		{
		case ETYPE_BOSS_PROTOTYPE:
			prototypeUnload( (boss_prototype*)e );
		}
		e = enemyUnregister( 0, e );
	}
}

void enemyRegister( enemy* e, int type )
{
	collisionRegister( &e->cb );
	e->type = type;
	e->next=0;
	if( !enemy_last )
	{
		enemy_first	= e;
		enemy_last	= e;
	}
	else
	{
		enemy_last->next	= e;
		enemy_last			= e;
	}
}

enemy* enemyUnregister( enemy* prev, enemy* e )
{
	collisionUnregister( &e->cb );
	enemy* r;
	if( e == enemy_first )
	{
		enemy_first = e->next;
		r = e->next;
		if( !r )
			enemy_last = 0;
	}
	else
	{
		prev->next = e->next;
		if( e == enemy_last )
			enemy_last = prev;
		r = e->next;
	}
	free( e );
	return r;
}

void enemiesUpdate( void )
{
	enemy* e;
	enemy* prev=0;
	for( e = enemy_first; e ; )
	{
		if( e->flags & EFLAG_ACTIVE )
		{
			switch( e->type )
			{
			case ETYPE_BIGPLANE:
				bpFrame( (bigplane*)e );
				break;
			case ETYPE_HELI:
				heliUpdate( (helicopter*)e );
				break;
			case ETYPE_JET:
				lpUpdate( (lilplane*)e );
				break;
			case ETYPE_TANK:
				tankUpdate( (tank*)e );
				break;
			case ETYPE_POWERUP:
				powerupUpdate( (powerup*)e );
				break;
			case ETYPE_STRUCTURE:
				structureUpdate( (structure*)e );
				break;
			case ETYPE_BOSS_PROTOTYPE:
				prototypeUpdate( (boss_prototype*)e );
				break;
			case ETYPE_SMISSILE:
				smissileUpdate( (smissile*)e );
				break;
			case ETYPE_BIGTANK:
				bigtankUpdate( (boss_bigtank*)e );
				break;
			case ETYPE_TOM:
				tomUpdate( (tom_cruise*)e );
				break;
			case ETYPE_DMISSILE:
				dmissileUpdate( (dmissile*)e );
				break;
			case ETYPE_TOMLASER:
				tomlaserUpdate( (tomlaser*)e );
				break;
			case ETYPE_TOMMYSTAR:
				tommystarUpdate( (tommystar*)e );
				break;
			case ETYPE_INVADER:
				invaderUpdate( (invader*)e );
			}
		}
		else
		{
			e = enemyUnregister( prev, e) ;
			continue;
		}
		prev = e;
		e = e->next;
	}
}

void enemiesDraw( void )
{
	enemy* e;
	for( e = enemy_first; e ; e = e->next )
	{
		if( e->flags & EFLAG_ACTIVE )
		{
			switch( e->type )
			{
			case ETYPE_BIGPLANE:
				bpDraw( (bigplane*)e );
				break;
			case ETYPE_HELI:
				heliDraw( (helicopter*)e );
				break;
			case ETYPE_JET:
				lpDraw( (lilplane*)e );
				break;
			case ETYPE_TANK:
				tankDraw( (tank*)e );
				break;
			case ETYPE_POWERUP:
				powerupDraw( (powerup*)e );
				break;
			case ETYPE_STRUCTURE:
				structureDraw( (structure*)e );
				break;
			case ETYPE_BOSS_PROTOTYPE:
				prototypeDraw( (boss_prototype*)e );
				break;
			case ETYPE_SMISSILE:
				smissileDraw( (smissile*)e );
				break;
			case ETYPE_BIGTANK:
				bigtankDraw( (boss_bigtank*)e );
				break;
			case ETYPE_TOM:
				tomDraw( (tom_cruise*)e );
				break;
			case ETYPE_DMISSILE:
				dmissileDraw( (dmissile*)e );
				break;
			case ETYPE_TOMLASER:
				tomlaserDraw( (tomlaser*)e );
				break;
			case ETYPE_TOMMYSTAR:
				tommystarDraw( (tommystar*)e );
				break;
			case ETYPE_INVADER:
				invaderDraw( (invader*)e );
			}
		}
	}
}
