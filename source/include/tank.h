#ifndef TANK_H
#define TANK_H

typedef struct   ttank
{
	struct ttank* next;
	u8 flags;
	u8 type;
	u16 script_read;
	collbox cb;
	s16 x;
	s16 y;
	u8 moving;
	u8 flash;
	u8 life;
	u8 a_x;
	u8 a_y;
	u8 a_mode;
	u8 t_angle;
	u8 sdelta;
	u8 angle;
	u8	reserved;
	u8	reserved1;
	u8	reserved2;
} tank;

//void tanksInit( void );
void tankSpawn( int x, int y, u16 script_read );
//tank* tankRemoveP( tank* p, tank* t );
void tankUpdate( tank* t );
void tankDraw( tank* t );

#endif
