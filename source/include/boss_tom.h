#ifndef BOSS_TOM_H
#define BOSS_TOM_H

#include <maxmod.h>

typedef struct ttom_piece
{	
	s16 x;		// horizontal coordinate
	s16 y;		// vertical coordinate
	s16	z;		// depth coordinat
	s16	vy;		// vertical velocity
	s8	vx;		// horizontal velocity
	
	u8	vz;
	u8	rot;	// rotation angle
	s8	vr;		// rotation velocity
	
} tom_piece;

typedef struct ttom_cruise
{
	u32		next;
	u8		flags;
	u8		type;
	s16		hp;
	collbox	cb;
	int		x;
	int		y;
	u8		mouth;
	u8		flash;
	u8		stage;
	u16		timer1;
	u8		timer2;
	
	u8		bounce_y;
	u8		bounce_x;
	
	u8		mstage;
	u16		mtimer;
	
	u8		death_flash;
	
	u8		laser_active;
	u8		laser_flash;
	u16		laser_length;
	
	u8		dead;
	mm_sfxhand	sfx_laser;
	
	int		disp_scale;
	int		disp_t;
	int		disp_f;
	
	tom_piece	pieces[88];
} tom_cruise;

typedef struct ttomlaser
{
	u32 next;
	u8	flags;
	u8	type;
	collbox	cb;
	s16	x;
	s16	y;
} tomlaser;

typedef struct ttommystar
{
	u32 	next;
	u8		flags;
	u8		type;
	collbox cb;
	int		x;
	int		y;
	u8		rot2;
	u8		dir;
	
	u16		rot;
} tommystar;

void SPAWN_TOM();
void tomUpdate( tom_cruise* t );
void tomDraw( tom_cruise* t );

void tomlaserSpawn( int x);
void tomlaserUpdate( tomlaser* t );
void tomlaserDraw( tomlaser* t );

void tommystarSpawn( int x, int y, int dir );
void tommystarUpdate( tommystar* t );
void tommystarDraw( tommystar* t );

#endif
