#include <gba.h>
#include "textlayer.h"
#include "ingame.h"

#include <maxmod.h>
#include "mmsolution.h"

#define DELAY_BEFORE_TITLE	60*1

u8 ask_continue;

void storyPlay( const char* story )
{
	int delay;
	REG_BG1CNT = SCREEN_BASE( 0xd000/0x800 ) | CHAR_BASE( 3);
	textInit( 0xd000/0x800, 3 );
	textDraw( 2, 2, 45, (char*)story );
	
	int p;
	for( p = 0; p < 256; p++ )
	{
		BG_PALETTE[p]=0;
		SPRITE_PALETTE[p]=0;
	}
	
	
	for( delay = 0; delay < 30; delay++ )
		VBlankIntrWait();
		
	if( !game_over )
		mmStart( MOD_FART, MM_PLAY_LOOP );
	else
		mmStart( MOD_GAMEOVER, MM_PLAY_LOOP );
	game_over=0;
	for( delay = 0; delay < 120; delay++ )
		VBlankIntrWait();
	
	VBlankIntrWait();
	
	BG_PALETTE[2]=0x7FFF;
	BG_PALETTE[3]=0x7FFF;
	BG_PALETTE[4]=0x7FFF;
	
	SetMode( MODE_0 |  BG1_ON );
	
	int timeout=60*5;
	int keysd=0;
	scanKeys();
	
	while( (!(keysd & KEY_START)) && (!(keysd & KEY_A)) && timeout )
	{
		VBlankIntrWait();
		textUpdate();
		scanKeys();
		keysd = keysDown();
		if( !text_write_active )
		{
			timeout--;
		}
	}
	mmStop();
	
	textClear( 0, 0, 1024 );
	
	if( ask_continue )
	{
		int bb=1;
		textDraw( 2,2, 50, "Continue?\rA: Yes Please!\rB: No! I can't handle\r   this game! :(" );
		while( bb )
		{
			VBlankIntrWait();
			textUpdate();
			scanKeys();
			//if( !text_write_active )
			{
				int keysd = keysDown();
				if( keysd & KEY_A )
				{
					mmStartSub( MOD_JCLICK );
					ask_continue=1;
					bb=0;
				}
				else if( keysd & KEY_B )
				{
					ask_continue=0;
					bb=0;
				}
			}
		}
	}
	SetMode( 0 );
	
	{
		int delay1;
		for( delay1= 0; delay1 < DELAY_BEFORE_TITLE; delay1++ ) VBlankIntrWait();
	}
	
}
