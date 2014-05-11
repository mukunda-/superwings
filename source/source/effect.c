#include <gba.h>
#include <stdlib.h>
#include "sprites.h"
#include "effect.h"
#include "level.h"

#include <maxmod.h>
#include "mmsolution.h"

#define MAX_EFFECTS 50
effectobject effects[MAX_EFFECTS];
int next_effect;

mm_sfxhand effectMediumExplosion( int x, int y )
{
	effectStart( x-8+(rand()&15),y-8+(rand()&15),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0);
	effectStart( x-8+(rand()&15),y-8+(rand()&15),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,-rand()&63);
	effectStart( x-8+(rand()&15),y-8+(rand()&15),0,0,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,-rand()&63);
	return mmEffect( SFX_EXPLOSION1 );
}

mm_sfxhand effectMediumExplosionS( int x, int y )
{
	effectStart( x-8+(rand()&15),y-8+(rand()&15),0,-level_scroll_speed,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,0);
	effectStart( x-8+(rand()&15),y-8+(rand()&15),0,-level_scroll_speed,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,1,-rand()&63);
	effectStart( x-8+(rand()&15),y-8+(rand()&15),0,-level_scroll_speed,effect_explosion_start,effect_explosion_speed,1,effect_explosion_len,0,-rand()&63);
	return mmEffect( SFX_EXPLOSION1 );
}


void effectsInit( void )
{
	int x;
	for( x = 0; x < MAX_EFFECTS; x++ )
		effects[x].active=0;
}

void effectStart( int x, int y, int vx, int vy, int framestart, int framespd, int framesize, int framelen, int scale, int frame )
{
	next_effect++;
	if( next_effect >= MAX_EFFECTS )next_effect=0;
	effects[next_effect].x=x<<7;
	effects[next_effect].y=y<<7;
	effects[next_effect].vx =vx;
	effects[next_effect].vy=vy;
	effects[next_effect].frame=frame;
	effects[next_effect].framestart = framestart;
	effects[next_effect].framespd = framespd;
	effects[next_effect].framesize = framesize;
	effects[next_effect].framelen = framelen;
	effects[next_effect].active = 1;
	effects[next_effect].scale = scale;
}

void effectsUpdate(void )
{
	int x;
	for( x = 0; x < MAX_EFFECTS; x++ )
	{
		if( effects[x].active )
		{
			effects[x].frame += effects[x].framespd;
			if( (effects[x].frame>>4) >= effects[x].framelen )
				effects[x].active=0;
			effects[x].x  += effects[x].vx;
			effects[x].y += effects[x].vy + level_scroll_speed;
		}
	}
}

void effectsDraw( void )
{
	int x;
	sprite *s;
	int w;
	for( x = 0; x < MAX_EFFECTS; x++ ) 
	{
		if( effects[x].active )
		{
			if( effects[x].frame >= 0 )
			{
				w = effects[x].scale ? 8 : 4;
				w <<= effects[x].framesize;
				s = spriteAdd( (effects[x].x>>7)-w, (effects[x].y>>7)-w, effects[x].framestart + ((effects[x].frame>>4) << effects[x].framesize), effects[x].framesize, 2 );
				if( effects[x].scale )
					spriteEnableRotation( s, 16, 1 );
			}
		}
	}
}
