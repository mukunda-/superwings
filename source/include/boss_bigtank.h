#ifndef BOSS_BIGTANK_H
#define BOSS_BIGTANK_H

#include "collision.h"

typedef struct boss_bigtank
{
	u32 next;
	u8	flags;
	u8	type;
	u16	hp;
	collbox	cb;
	
	int x;
	int y;
	
	u16	angle;
	
	u8	t_angle;
	u8	flash;
	u8	stage;
	///////////////////
	
	int speed;
	int duration;
	int	m_speed;
	
	u8	m_angle;
	u8	t_x;
	u8	t_y;
	//////////////////
	
	u8	s_missile;
	u8	launch_switch;
	
	u16	music_fade;
	
} boss_bigtank;

void bigtankSpawn();
void bigtankUpdate( boss_bigtank* t );
void bigtankDraw( boss_bigtank* t );

#endif
