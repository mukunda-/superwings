#ifndef COLLISION_H
#define COLLISION_H

typedef struct  tcollbox 
{
	struct tcollbox* next;
	s16 x;
	s16 y;
	u8 damage;
	u8 w;
	u8 h;
	u8 t:4;
	u8 src:4;
} collbox;

#define CTYPE_X			3
#define CTYPE_POWERUP	2
#define CTYPE_ENEMY		1
#define CTYPE_PLAYER	0

#define CSRC_HUH		0
#define CSRC_P1			1
#define CSRC_P2			2
#define CSRC_ENEMY		3

void collisionInit( void );
void collisionFlush( void );
void collisionRegister( collbox* p_cb );
void collisionUnregister( collbox* p_cb );
collbox* collisionCheck( int x, int y, int t );
void collisionHit( collbox* p_cb, int damage, u8 src );
collbox* collisionFind( int x1, int y1, int x2, int y2, int t );
void collisionDamageArea( int x1, int y1, int x2, int y2, int t, int damage, int src );
void addPoints( int dest, int points );

#endif
