#ifndef ENEMEIS_H
#define ENEMEIS_H

#include "collision.h"

typedef struct  tenemy
{
	struct tenemy* next;
	u8	flags;
	u8	type;
	u8	reserved2;
	u8	reserved3;
	collbox	cb;
} enemy;

// look at sdata_sizes before changes
#define ETYPE_BIGPLANE	1
#define ETYPE_HELI		2
#define ETYPE_JET		3
#define ETYPE_TANK		4
#define ETYPE_POWERUP	5
#define ETYPE_STRUCTURE	6
#define ETYPE_BOSS_PROTOTYPE	7
#define ETYPE_SMISSILE	8
#define ETYPE_BIGTANK	9
#define ETYPE_TOM		10
#define ETYPE_DMISSILE	11
#define ETYPE_TOMLASER	12
#define ETYPE_TOMMYSTAR 13
#define ETYPE_INVADER	14

#define EFLAG_ACTIVE	1
#define EFLAG_SCRIPT	2
#define EFLAG_CUSTOM1	4
#define EFLAG_CUSTOM2	8
#define EFLAG_CUSTOM3	16
#define EFLAG_CUSTOM4	32
#define EFLAG_CUSTOM5	64

void enemiesInit(  void );
void enemiesCleanup( void );
void enemyRegister( enemy* e, int type );
enemy* enemyUnregister( enemy* prev, enemy* e );
void enemiesUpdate( void );
void enemiesDraw( void );

extern enemy *enemy_first;
extern enemy *enemy_last;

#endif
