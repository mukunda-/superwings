#ifndef BIGPLANE_H
#define BIGPLANE_H

typedef struct tbigplane
{
	struct tbigplane* next;
	u8 flags;
	u8 type;
	u8 life;
	u8 angle;
	collbox cb;
	
	s16 x;
	s16 y;
	u16 script_read;
	u8 speed;
	
	u8 flash;
	u8 sdelta;
	u8 frame;
	
	u8 firedir;
	u8 firetmr;
	u8 fireshots;
	u8 Elmo;
	
} bigplane;

//void bpInit( void );
//void bpCleanup( void );
void bpSpawn( int x, int y, u16 script_read );
//bigplane* bpRemove( bigplane* prev, bigplane* p );
void bpFrame( bigplane* p );
void bpDraw( bigplane* p );

#endif
