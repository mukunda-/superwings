#ifndef POWERUP_H
#define POWERUP_H

typedef struct tpowerup
{
	struct tpowerup* next;
	u8	flags;
	u8	type;
	u8	life;
	u8	x;
	collbox	cb;
	u8	y;
	u8	flash;
	u8	reserved;
	u8	item;
} powerup;

void powerupSpawn( int x, int y, int item );
void powerupUpdate( powerup* p ) ;
void powerupDraw( powerup* p );

#define POWERUP_POWER	1
#define POWERUP_BOMB	2
#define POWERUP_1UP		3

#endif
