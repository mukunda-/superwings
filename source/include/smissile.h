#ifndef sMiSsIlE_H
#define sMiSsIlE_H

typedef struct  tsmissile
{
	struct tsmissile* next;
	u8 flags;
	u8 type;
	u8 stage;
	u8 vel;
	collbox cb;
	s16 x;
	s16 y;
} smissile;

void smissileSpawn( int x, int y );
void smissileUpdate( smissile* s );
void smissileDraw( smissile* s );

#endif
