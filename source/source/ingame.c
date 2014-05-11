#include <gba.h>
#include "ingame.h"
#include "collision.h"
#include "level1.map.h"
#include "level2.map.h"
#include "level3.map.h"
#include "level.h"
#include "plane.h"
#include "sprites.h"
#include "bullet.h"
#include "heli.h"
#include "sprites.h"
#include "effect.h"
#include "maxmod.h"
#include "mmsolution.h"
#include "tank.h"
#include "bigplane.h"
#include "textlayer.h"
#include "lilplane.h"
#include "enemies.h"
#include "structure.h"
#include "boss_prototype.h"
#include <stdlib.h>

#include "main.h"
#include "comms.h"
#include "multiboot.h"
#include "lvlscripts.h"

#define fadein_spd 50
#define autofire_threshold 5

#define RESPAWN_TIME 50

#include "gfx_bigtank.h"


int detect_autofire=0;
int afguess=0;

int p1_score;
int p2_score;

int game_fade;
int game_fademode;
u8	game_fade_dirty;

u8	screen_flashing=0;

u8	P1_LIVES;
u8	P2_LIVES;

u8	stats_dirty_p1;
u8	stats_dirty_p2;

u8	game_paused;

u8	game_over;

u8	credits;

void flashScreen( int flash_time )
{
	screen_flashing = flash_time;
}

void record_autofire()
{
	int keysh = keysHeld();
	if( !afguess )
	{
		if( keysh & KEY_B )
		{
			detect_autofire++;
			afguess=1;
		}
		else
			detect_autofire=0;
	}
	else
	{
		if( !(keysh & KEY_B) )
		{
			detect_autofire++;
			afguess=0;
		}
		else
			detect_autofire=0;
	}
}

void check_cheats( void )
{
	
	record_autofire();
	while( detect_autofire > autofire_threshold )
	{
		VBlankIntrWait();
		scanKeys();
		record_autofire();
	}
}

void tryMultiboot( void )
{
	if( !comm_connection )
	{
		if( commsCheckConnection() == 0x7202 )
		{
			// disable maxmod :| there should be a safer function for this soon
//			irqDisable( IRQ_VCOUNT );
			irqDisable( IRQ_VBLANK );
			REG_DMA1CNT=0;
			REG_DMA2CNT=0;
			
			// multiboot image
			commsMultiboot();
			
			// begin communication
			commBegin();
			
			// enable maxmod
			mmVBlank();
			mmVBlank();
//			irqEnable( IRQ_VCOUNT );
			irqEnable( IRQ_VBLANK );
		
			// wait for new frame and continue
			VBlankIntrWait();
		}
	}
}

void slaveBegin( void )
{
	// disable maxmod
//	irqDisable( IRQ_VCOUNT );
	irqDisable( IRQ_VBLANK );
	REG_DMA1CNT=0;
	REG_DMA2CNT=0;
	
	// start comms
	commBegin();
	
	// enable maxmod
	mmVBlank();
	mmVBlank();
//	irqEnable( IRQ_VCOUNT );
	irqEnable( IRQ_VBLANK );
	
	// wait for new frame
	VBlankIntrWait();
}

void planeInput( plane* p, int keysh, int keysd )
{
	if( keysh & KEY_LEFT )
		planeSteerLeft( p );
	if( keysh & KEY_RIGHT )
		planeSteerRight( p );
	if( keysh & KEY_UP )
		planeAccel( p );
	if( keysh & KEY_DOWN )
		planeSlow( p );
	if( keysd & KEY_B )
		planeShoot( p );
	if( keysd & KEY_A )
		planeDropBomb( p );
}

void drawP1stats( void )
{
	if( stats_dirty_p1 )
	{
		stats_dirty_p1=0;
		int tx;
		textDrawImm( 1,1, "P1" );
		for( tx = 0; tx < p1.missiles; tx+=2 )
			textDrawChar( 1+(tx>>1),19, 0x04 );
		if( tx > p1.missiles )
			textDrawChar( 1+(tx>>1)-1,19, 0x05 );
		textDrawImm( 1+(tx>>1),19, "     " );
		
		if( p1_score != p1.score )
		{
			stats_dirty_p1=1;
			int d;
			for( d = 0; d < 8; d++ )
			{
				if( (p1_score & (0xF<<(d<<2))) != (p1.score & (0xF<<(d<<2))) )
				{
					int n = (p1_score>>(d<<2) & 0xF);
					n++;
					if( n > 9 ) n =0;
					p1_score = (p1_score & ~(0xF<<(d<<2))) | (n<<(d<<2));
					break;
				}
			}
			
		}
		textDrawNumber( 14, 1, 8, p1_score );
		
		for( tx = 0; tx < P1_LIVES; tx++ )
			textDrawChar( 1 + tx, 2, 3 );
		for( ; tx < 8; tx++ )
			textDrawChar( 1 + tx, 2, 0 );
	}
}

void drawP2stats( void )
{
	if( stats_dirty_p2 )
	{
		stats_dirty_p2 = 0;
		if( comm_connection )
		{
		
			int tx;
			textDrawImm( 1+15,1, "P2" );
			for( tx = 0; tx < p2.missiles; tx+=2 )
				textDrawChar( 29-(tx>>1),19, 0x04 );
			if( tx > p2.missiles )
				textDrawChar( 29-(tx>>1)+1,19, 0x07 );
			textDrawImm( 29-(tx>>1)-4,19, "     " );
			
			if( p2_score != p2.score )
			{
				stats_dirty_p2 = 1;
				int d;
				for( d = 0; d < 8; d++ )
				{
					if( (p2_score & (0xF<<(d<<2))) != (p2.score & (0xF<<(d<<2))) )
					{
						int n = (p2_score>>(d<<2) & 0xF);
						n++;
						if( n > 9 ) n =0;
						p2_score = (p2_score & ~(0xF<<(d<<2))) | (n<<(d<<2));
						break;
					}
				}
			}
			textDrawNumber( 15+14, 1, 8, p2_score );
			
		//	textDrawImm( 23, 2, "    \x03\x03\x03" );
			for( tx = 0; tx < P1_LIVES; tx++ )
				textDrawChar( 29 - tx, 2, 3 );
			for( ; tx < 8; tx++ )
				textDrawChar( 29 - tx, 2, 0 );
		}
		else
		{
			textDrawImm( 1+15, 1, "P2    CONNECT" );
			textDrawImm( 1+15, 2, "       CABLE" );
		}
	}
}

void resetGame( void )
{
	credits = 2;
	P1_LIVES = 2;
	P2_LIVES = 0;
	p1.score = 0;
	p2.score = 0;
	game_paused=0;
	game_over=0;
}

//-------------------------------------------------------------------------------------------------
void playLevel( int level )
//-------------------------------------------------------------------------------------------------
{
	//------------------------------------
	// variables
	//------------------------------------
	
	int keysh = 0;
	int keysd = 0;
	int p1_respawn = RESPAWN_TIME;
	int p2_respawn = RESPAWN_TIME;
	game_fade = 16<<8;
	game_fademode = fademode_in;
	u8  level_running = 1;
	
	//------------------------------------
	
	// init level stuff...
	levelInit();
	
	
	irqDisable( IRQ_VCOUNT );
	mmVolume( 1024 );
	// load level
	switch( level )
	{
	case 0:
		levelLoad( (void*)level1_mapTiles, (u16*)level1_mapPal, 206, (void*)level1_mapMap, script1, 0 );
		mmStart( MOD_LEVEL1, MM_PLAY_LOOP );
		//mmStart( MOD_BOSS1, MM_PLAY_LOOP );
		break;
	case 1:
		levelLoad( (void*)level2_mapTiles, (u16*)level2_mapPal, (1816/8)-2, (void*)level2_mapMap, script2, 0 );
		mmStart( MOD_LEVEL2, MM_PLAY_LOOP );
		break;
	case 2:
		levelLoad( (void*)level3_mapTiles, (u16*)level3_mapPal, (512/8), (void*)level3_mapMap, script3, 0 );
		//mmStart( MOD_BOSS3, MM_PLAY_LOOP );
		break;
	}
	
	// switch to real mode
	levelSwitchMode( 0 );
	
	// generate sprite affine table
	spritesSetupAffine();
	
	// clear effects
	effectsInit();
	
	// init text layer
	textInit( 0xd000/0x800, 3 );
	
	if( comm_master )
	{
		planeReset( &p1, 80, 0 );
	//	planeReset( &p2, 160, 1 );
	}
	else
	{
	//	planeReset( &p1, 80, 1 );
	//	planeReset( &p2, 160, 0 );
	}
	//bulletsInit();
	bulletsClear();

	
	// setup text layer
	REG_BG1CNT = SCREEN_BASE( 0xd000/0x800 ) | CHAR_BASE( 3);
	REG_BG1HOFS=4;
	REG_BG1VOFS=4;
	REG_BG2HOFS=3;
	REG_BG2VOFS=3;
	REG_BG2CNT = SCREEN_BASE( 0xd800/0x800 ) | CHAR_BASE( 3);
	
	// p2 == connect cable??
	
/*	textDrawImm( 1, 1, "P1    00000000" );
	textDrawImm( 1+15, 1, "P2    CONNECT" );
	textDrawImm( 1+15, 2, "       CABLE" );*/
	
	stats_dirty_p1 = 1;
	stats_dirty_p2 = 1;
	
	spritesEraseOAM();
	
	copyPalette(-4);
	
	VBlankIntrWait();
	SetMode( MODE_0 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON );
	
//	if( !comm_master )
//	{
//		slaveBegin();
//		//fade=0;
//	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//LZ77UnCompVram( (void*)gfx_bigtankTiles, (void*)0x6014800 );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//SPAWN_TOM();
	//bigtankSpawn();
	//prototypeSpawn();
	
	while( level_running )
	{
		// generate random numbers
		rand();
		
		// scan keypad input
		scanKeys();
		
		// process maxmod frame
		mmFrame();
		
		// read keypad state
		keysh=keysHeld();
		keysd=keysDown();
		
		if( keysd & KEY_START )
		{
			mmStartSub( MOD_PAUSE );
			game_fade_dirty = !game_fade_dirty;
			game_paused = !game_paused;
			if( game_paused )
			{
				mmPause();//mmVolume( 500 );
			}
			else
			{
				mmResume();//mmVolume( 1024 );
			}
		}
		
		if( !game_paused )
		{
			if( comm_master )
			{
				planeInput( &p1, keysh, keysd );
			}
	//		else
	//			planeInput( &p2, keysh, keysd );
			
			if( !comm_connection )
				check_cheats();	// disable autofire ;)
			
			// attempt to transfer game to slave (if any)
			tryMultiboot();
			
			levelUpdate();
			
			if( !p1.active )
			{
				p1_respawn--;
				if( p1_respawn == 0 )
				{
					p1_respawn = RESPAWN_TIME;
					if( P1_LIVES )
					{
						planeReset( &p1, 80, 0 );
						P1_LIVES--;
						stats_dirty_p1 = 1;
					}
				}
			}
			if( !p2.active )
			{
				p2_respawn--;
				if( p2_respawn == 0 )
				{
					p2_respawn = RESPAWN_TIME;
					if( P2_LIVES )
					{
						planeReset( &p2, 160, 1 );
						P2_LIVES--;
						stats_dirty_p2 = 1;
					}
				}
			}
			if( !game_over )
			{
				if( (!p1.active) && (!p2.active) )
				{
					if( (!P1_LIVES) && (!P2_LIVES) )
					{
						game_over=1;
						levelEnd();
					}
				}
			}
			
			// refresh other game objects
			bulletsRefresh();
			enemiesUpdate();
			planeUpdate( &p1 );
			planeUpdate( &p2 );
			effectsUpdate();
			
			// fade screen...
			
			
			
			// SLAVE: setup data to be transferred
			// MASTER: setup data to be transferred and exchange data
			commTransferData();
		}
		else
		{
			
		}
		
		VBlankIntrWait();
		
		if( screen_flashing )
		{
			screen_flashing--;
			copyPalette( screen_flashing >>1);
		}
		else
		{
			if( game_fademode || game_fade_dirty )
			{
				game_fade_dirty=0;
				switch( game_fademode )
				{
				case fademode_in:
					game_fade-=fadein_spd;
					if( game_fade < 0 )
					{
						game_fade = 0;
						game_fademode = fademode_none;
					}
					break;
				case fademode_out:
					game_fade+=fadein_spd;
					if( game_fade >= 16<<8 )
					{
						game_fade = 16<<8;
						game_fademode = fademode_none;
						level_running = 0;
					}
					mmVolume( ((16<<8)-game_fade) >> 2 );
					break;
				case fademode_end:
					game_fade++;
					if( game_fade >= 100 )
					{
						game_fademode = fademode_out;
						game_fade = 0;
					}
					break;
				}
				if( game_fademode != fademode_end )
				{
					
					copyPalette( 0 - (game_fade>>10) - (game_paused?1:0) );
				}
				
			}
		}
		
		levelDraw();
		if( !game_paused )
			textUpdate();
		
		drawP1stats();
		drawP2stats();
		
		{
			// copy text to shadow
			int tx;
			for( tx =0 ; tx < 32*19; tx++ )
				((u16*)0x600d800)[tx] =((u16*)0x600d000)[tx] + (4<<12);
		}
		
		spritesBegin();
		planeDraw( &p1, 30, comm_master ? 0 : 7 );
		planeDraw( &p2, 31, comm_master ? 7 : 0 );
		
		enemiesDraw();
		
		bulletsDraw();
		effectsDraw();
		spritesEnd();
	}
	
	enemiesCleanup();
	mmStop();
	mmVolume( 1024 );
	irqEnable( IRQ_VCOUNT );
}
