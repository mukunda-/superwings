#ifndef INVADER_H
#define INVADER_H

typedef struct
{
	u32 next;
	u8 flags;
	u8 type;
	u8 species;
	u8 hp;
	collbox cb;
	s16 x;
	s16 y;
	u8 dir;
	u8 shot;
	u8	frame;
	u8	r3;
	
} invader;

void invaderSpawn( int x, int y );
void invaderUpdate( invader* p );
void invaderDraw( invader* p );

#endif
