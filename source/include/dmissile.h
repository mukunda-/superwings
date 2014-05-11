#ifndef DMISSILE_H
#define DMISSILE_H

#include "collision.h"

// dmissile for bigtank

typedef struct tdmissile
{
	u32 next;
	u8 flags;
	u8 type;
	u8 smoke;
	u8 dir;
	collbox cb;
	s16 x;
	s16 y;
	s16 z;
	s16 vel;
} dmissile;

void dmissileSpawn( int x, int y, u8 dir );
void dmissileUpdate( dmissile* m );
void dmissileDraw( dmissile* m );

#endif
