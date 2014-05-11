#include <gba.h>
#include <stdlib.h>

#include "level.h"
#include "ingame.h"
#include "heli.h"
#include "tank.h"
#include "bigplane.h"
#include "lilplane.h"
#include "textlayer.h"
#include "nocash.h"
#include "main.h"
#include "structure.h"
#include "boss_prototype.h"
#include "boss_bigtank.h"
#include "boss_tom.h"
#include "invader.h"

#include <maxmod.h>
#include "mmsolution.h"

#define bgmap	((u16*)0x600e000)

#define LVLCMD_NULL 	0
#define LVLCMD_MESSAGE 	1
#define LVLCMD_SPAWN 	2
#define LVLCMD_END 		3
#define LVLCMD_SPEED 	4

#define STYPE_HELI 				1
#define STYPE_TANK 				2
#define STYPE_BIGPLANE 			3
#define STYPE_JET 				4
#define STYPE_STRUC_WAREHOUSE 	5
#define STYPE_STRUC_BARREL 		6
#define STYPE_STRUC_BARRELS 	7
#define STYPE_STRUC_CRATE 		8
#define STYPE_STRUC_CRATES 		9
#define STYPE_PROTOTYPE 		10
#define STYPE_STRUC_BIGHOUSE	11
#define STYPE_BIGTANK			12
#define STYPE_TOM				13
#define STYPE_INVADER			14

#define levelMap	((u16*)0x6008000)


#define TEXT_LENGTH	(32*8)
#define TEXT_SHOW_TIME 200

u16 n_aiscripts;
u8* n_aiscript_off;
u8* levelScript;

u32 level_size;
u8 validscript;

int level_pos;
int scroll_pos;
int delta_counter;
int script_pos;

int level_scroll_speed;

int text_show;

u8 demoSwitch;


void levelInit( void )
{
	REG_BG0CNT = SCREEN_BASE(0xe000/0x800) | BG_PRIORITY(3);
}

void levelPatch( void )
{
	// patch screen with full bg
	int x;
	int y;
	scroll_pos &= (1<<11)-1;
	u16* data;
	data = levelMap+ 30*level_size-20*30-(level_pos>>11);
	for( x =0;x<30;x++)
		for(y=0;y<20;y++)
		{
			bgmap[x+y*32] = data[x+y*30];
		}
	REG_BG0VOFS = -(scroll_pos>>8);
}

void levelLoad( void* tiles, u16* palette, int y_size, void* map, void* script, int pos )
{
	// copy palettes
	
	setPalette( palette+1 );
	copyPalette( 0 );
	// palette[0] must equal palette[1]
	// copy tiles to vram
	LZ77UnCompVram( tiles, (void*)0x6000000 );
	LZ77UnCompVram( map, (void*)levelMap );
	
	if( script != 0 )
	{
		levelScript = (u8*)script;
		validscript=1;
		script_pos=0;
		
		n_aiscripts = levelScript[0] + (levelScript[1]<<8);
		levelScript += 2;
		n_aiscript_off = levelScript;
		levelScript += 2*n_aiscripts;
	}
	level_scroll_speed=40;
	
	level_size = y_size;
	
	level_pos=pos;
	delta_counter=pos;
	
	levelPatch();
	scroll_pos=0;
	REG_BG0VOFS=0;
}

void levelWriteLine( void )
{
	u16* bgd;
	bgd = bgmap + (((u32)(-(scroll_pos>>11)-1))&31)*32;
	u16* data;
	data = levelMap+ 30*level_size-20*30-((level_pos>>11)-1)*30;
	
	int x;
	for( x = 0;x<30; x++ )
		bgd[x] = data[x];
}

void runScriptCommand( void )
{
	int pa,pb,pc,pd;
	switch( levelScript[script_pos++] )
	{
	case LVLCMD_NULL:
		break;
	case LVLCMD_MESSAGE:
		
		textDraw( TEXT_START_X,TEXT_START_Y,TEXT_SPEED,(char*)(levelScript+script_pos+1) );
		script_pos += levelScript[script_pos]+1;
		text_show = TEXT_SHOW_TIME;
		
		break;
	case LVLCMD_SPAWN:
		pa = (s16)(levelScript[script_pos+0] + (levelScript[script_pos+1]<<8));
		pb = (s16)(levelScript[script_pos+2] + (levelScript[script_pos+3]<<8));
		pc = levelScript[script_pos+4];
		pd = levelScript[script_pos+5];
		
		script_pos += 6;
		// spawn enemy
		switch( pc )
		{
		case STYPE_HELI:
			heliSpawn( pa,pb, n_aiscript_off[(pd<<1)] + (n_aiscript_off[1+(pd<<1)]<<8) - (n_aiscripts*2+2));
			break;
		case STYPE_TANK:
			tankSpawn( pa, pb, n_aiscript_off[(pd<<1)] + (n_aiscript_off[1+(pd<<1)]<<8) - (n_aiscripts*2+2) );
			break;
		case STYPE_BIGPLANE:
			bpSpawn( pa, pb, n_aiscript_off[(pd<<1)] + (n_aiscript_off[1+(pd<<1)]<<8) - (n_aiscripts*2+2) );
			break;
		case STYPE_JET:
			lpSpawn( pa, pb, n_aiscript_off[(pd<<1)] + (n_aiscript_off[1+(pd<<1)]<<8) - (n_aiscripts*2+2) );
			break;
		case STYPE_STRUC_WAREHOUSE:
		case STYPE_STRUC_BARREL:
		case STYPE_STRUC_BARRELS:
		case STYPE_STRUC_CRATE:
		case STYPE_STRUC_CRATES:
			structureSpawn( pa, pb, pc - STYPE_STRUC_WAREHOUSE, n_aiscript_off[(pd<<1)] + (n_aiscript_off[1+(pd<<1)]<<8) - (n_aiscripts*2+2)  );
			break;
		case STYPE_PROTOTYPE:
			prototypeSpawn();
			mmStart( MOD_BOSS2, MM_PLAY_LOOP );
			break;
		case STYPE_STRUC_BIGHOUSE:
			structureSpawn( pa, pb, STRUCTURE_BIGHOUSE, n_aiscript_off[(pd<<1)] + (n_aiscript_off[1+(pd<<1)]<<8) - (n_aiscripts*2+2) );
			break;
		case STYPE_BIGTANK:
			bigtankSpawn();
		//	mmStart( MOD_BOSS1, MM_PLAY_LOOP );
			break;
		case STYPE_TOM:
			SPAWN_TOM();
			// play music
			break;
		case STYPE_INVADER:
			invaderSpawn( pa,pb );
		}
		break;
	case LVLCMD_END:
		validscript=0;
		break;
	case LVLCMD_SPEED:
		
		level_scroll_speed = levelScript[script_pos]*2;
		script_pos++;
		break;
	}
}

void parseLevelScript( void )
{
	if( validscript )
	{
		if( (delta_counter>>8) >= levelScript[script_pos] )
		{
			delta_counter -= levelScript[script_pos]<<8;
			script_pos++;
			runScriptCommand();
		}
	}
}

void levelUpdate()
{
	int amount = level_scroll_speed<<1;
//	int old_level_pos = level_pos;
	level_pos += amount;
	if( demoSwitch != 1 )
	{
		if( (level_pos >>11) > level_size-19 )
			level_pos -= 19<<11;
	}
	else
	{
		if( (level_pos >>11) > 19 )
			level_pos -= 19<<11;
	}
	delta_counter += amount;
	if( demoSwitch == 0 )
		parseLevelScript();
	scroll_pos += amount;
	
}

void levelDraw(void)
{
//	char a[5];
//	a[0] = '0' + (level_pos>>8)/100;
//	a[1] = '0' + (((level_pos>>8)/10)%10);
//	a[2] = '0' + ((level_pos>>8)%10);
//	a[3]=0;
//	textDrawImm(0,0,a );
	if( text_show )
	{
		if( !text_write_active )
		{
			text_show--;
			if( !text_show )
			{
				textClear( TEXT_START_X, TEXT_START_Y, TEXT_LENGTH );
			}
		}
	}
	levelWriteLine();
	REG_BG0VOFS = -(scroll_pos>>8);
}

void levelSwitchMode( int demo )
{
	demoSwitch = demo;
}

void levelEnd( void )
{
	game_fademode = fademode_end;
	
}
