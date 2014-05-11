#ifndef LILPLANE_H
#define LILPLANE_H

typedef struct   t_lilplane
{
	struct t_lilplane* next;
	u8 flags;
	u8 type;
	u16 script_read;
	collbox cb;
	s16 x;
	s16 y;
} lilplane;


//void lpInit( void );
//void lpCleanup( void );
void lpSpawn( int x, int y, u16 script_read );
//lilplane* lpRemove( lilplane* prev, lilplane* p );
void lpUpdate( lilplane* p );
void lpDraw( lilplane* p );

#endif
