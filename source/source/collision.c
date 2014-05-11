#include <gba.h>
#include "collision.h"
#include "plane.h"
#include "ingame.h"
#include "nocash.h"

collbox*	cb_start;
collbox*	cb_end;

void collisionInit( void )
{
	cb_start=0;
	cb_end=0;
}

void collisionRegister( collbox* p_cb )
{
	if( cb_end )
	{
		cb_end->next = p_cb;
	}
	else
	{
		cb_start = p_cb;
	}
	cb_end = p_cb;
	p_cb->damage=0;
	p_cb->w=0;
	p_cb->next=0;
}

void collisionUnregister( collbox* p_cb )
{
	collbox* r;
	
	if( p_cb == cb_start )
	{
		r = p_cb->next;
		cb_start = r;
		if( !r )
		{
			cb_end=0;
		}
	}
	else
	{
		collbox* c;
		collbox* p;
		p = cb_start;
		for( c = cb_start->next; c != 0; c=c->next )
		{
			if( c == p_cb )
			{
				if( c == cb_end )
					cb_end = p;
				p->next = c->next;
				break;
			}
			p=c;
		}
	}
	p_cb->next=0;
}

collbox* collisionCheck( int x, int y, int t )
{
	collbox* cb;
	cb = cb_start;
	while( cb != 0 )
	{
		if( x >= cb->x ) {
			
			if(  x < cb->x+cb->w ) {
				if( y >= cb->y ) {
					if( y < cb->y+cb->h ) {
						if( t == cb->t ) {
							
							return cb;
						}
					}
				}
			}
		}
		cb = cb->next;
	}
	return 0;
}

collbox* collisionFind( int x1, int y1, int x2, int y2, int t )
{
	collbox* cb;
	int cx,cy;
	for( cb = cb_start; cb ; cb = cb->next )
	{
		if( t == cb->t )
		{
			if( cb->w != 0 )
			{
				cx = cb->x+(cb->w>>1);
				cy = cb->y+(cb->h>>1);
				if( cx >= x1 )
					if( cx < x2 )
						if( cy >= y1 )
							if( cy < y2 )
								return cb;
			}
		}
	}
	return 0;
}

void collisionHit( collbox* p_cb, int damage, u8 src )
{
	p_cb->damage = (int)p_cb->damage+damage>0xFF ? 0xFF : p_cb->damage+damage;
	p_cb->src = src;
}

void collisionDamageArea( int x1, int y1, int x2, int y2, int t, int damage, int src )
{
	collbox* cb;
	int cx,cy;
	for( cb = cb_start; cb ; cb = cb->next )
	{
		if( t == cb->t )
		{
			if( cb->w != 0 )
			{
				cx = cb->x+(cb->w>>1);
				cy = cb->y+(cb->h>>1);
				if( cx >= x1 )
					if( cx < x2 )
						if( cy >= y1 )
							if( cy < y2 )
							{
								cb->damage += damage;
								cb->src = src;
							}
			}
		}
	}
}

int add16( int source, int amount )
{
	source += amount;
	int d;
	for( d = 0; d < 8; d++ )
	{
		if( ((source >> (d<<2)) & 0xF) >= 10 )
		{
			source -= 10<<(d<<2);
			source += 1<<((d+1)<<2);
		}
	}
	return source;
}

void addPoints( int dest, int points )
{
	int tpoints;
	tpoints=0;
	tpoints = (points % 10) + (((points/10)%10)<<4) + (((points/100)%10)<<8) + (((points/1000)%10)<<12) + (((points/10000)%10)<<16) + (((points/100000)%10)<<20);
	
	switch( dest )
	{
	case CSRC_P1:
		p1.score = add16( p1.score, tpoints );
		stats_dirty_p1 = 1;
		break;
	case CSRC_P2:
		p2.score = add16( p2.score, tpoints );
		stats_dirty_p2 = 1;
	}
}
