#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "collision.h"

typedef struct  tstructure
{
	struct tstructure* next;
	u8 flags;
	u8 type;
	u8 cls;
	u8 flash;
	collbox cb;
	s16 y;
	u8	x;
	u8	sdelta;
	u16 hp;
	u16 script_read;
} structure;

#define STRUCTURE_WAREHOUSE				0
#define STRUCTURE_BARREL				1
#define STRUCTURE_BARRELS				2
#define STRUCTURE_CRATE					3
#define STRUCTURE_CRATES				4
#define STRUCTURE_WAREHOUSE_DESTROYED	5
#define STRUCTURE_SMALL_RUBBLE			6
#define STRUCTURE_WIDE_RUBBLE			7
#define STRUCTURE_LARGE_RUBBLE			8
#define STRUCTURE_BIGHOUSE				9
#define STRUCTURE_BIGHOUSE_DESTROYED	10

#define STRUCTURE_SIZE_8x8		0	
#define STRUCTURE_SIZE_8x16		1
#define STRUCTURE_SIZE_16x16	2
#define STRUCTURE_SIZE_32x32	3
#define STRUCTURE_SIZE_32x40	4
#define STRUCTURE_SIZE_64x64	5

void structureSpawn( int x, int y, int cls, u16 script );
void structureUpdate( structure* s );
void structureDraw( structure* s );

#endif
