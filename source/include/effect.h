#ifndef owiehfowiehfwoiehfw
#define owiehfowiehfwoiehfw

#include <maxmod.h>

typedef struct teffect
{
	s16 x;
	s16 y;
	s16 vy;
	s8 vx;
	s8 frame;
	u8 framespd;
	u8 scale;
	u16 framestart;
	u8 framesize;
	u8 framelen;
	u8 active;
	u8 	r1;
} effectobject;

void effectsInit( void );
void effectStart( int x, int y, int vx, int vy, int framestart, int framespd, int framesize, int framelen, int scale, int frame );
void effectsUpdate(void );

void effectsDraw( void );

mm_sfxhand effectMediumExplosion( int x, int y );
mm_sfxhand effectMediumExplosionS( int x, int y );

#define effect_smoke_start	212
#define effect_smoke_len	4
#define effect_explosion_start	276
#define effect_explosion_speed	5
#define effect_explosion_len	6

#endif
