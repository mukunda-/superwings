// 2.03.08 - development begins

#include <gba.h>
#include <stdlib.h>
#include "intro.h"
#include "title.h"
#include "pilotselect.h"
#include "gamestart.h"
#include "ingame.h"
#include "nocash.h"
#include "maxmod.h"
#include "mmsolution.h"
#include "mmsolution_bin.h"
#include "multiboot.h"
#include "gfx_sprite.h"
#include "story.h"
#include "collision.h"
#include "enemies.h"
#include "bullet.h"
#include "plane.h"
#include "stringtable.h"

#define DELAY_BEFORE_TITLE	60*1
#define DELAY_BEFORE_PILOT	50*1

u8 mixbuffer[ MM_MIXLEN_16KHZ ] __attribute((aligned(4)));

u16* my_palette;

#define nchannels 16

u8 song_messages[16];

extern u8 credits;
extern u8 ask_continue;
extern int p1_score;
extern u8 P1_LIVES;

mm_word mmCallback( mm_word msg, mm_word param )
{
	switch( msg )
	{
	case MMCB_SONGMESSAGE:
		song_messages[param&15] = 1;
		break;
	}
	return 0;
}

void maxmodUpdate( void )
{
///	REG_IME=1;
	mmFrame();
}

void copyGraphics( void )
{
	// copy obj graphics
	LZ77UnCompVram( (void*)gfx_spriteTiles, (void*)0x6010000 );
}

void setPalette( u16* palette )
{
	my_palette = palette;
}

void copyPalette( int brightness )
{
	int p;
	int a;
	int b;
	for( p = 0; p < 4; p++ )
	{
		for( a = 0; a < 8; a++ )
		{
			if( a < 4 )
				b = p + a + brightness;
			else
				b = p - (8-a) + brightness;
			
			if( b < 0 ) b = 0;
			if( b > 3 ) b = 3;
			
			BG_PALETTE[    p+1+a*16] = my_palette[b];
			SPRITE_PALETTE[p+1+a*16] = my_palette[b];
		}
	}
	BG_PALETTE[0] = my_palette[0];
}

void setupMaxmod( void )
{
	u8* mmdata;
	mm_gba_system mmsystem;
	mm_addr mod_channels;
	mm_addr act_channels;
	mm_addr mix_channels;
	mm_addr wavebuffer;
	mmdata = (u8*)malloc( nchannels* (MM_SIZEOF_MODCH + MM_SIZEOF_ACTCH + MM_SIZEOF_MIXCH) + MM_MIXLEN_16KHZ );
	mod_channels = (mm_addr)mmdata;
	act_channels = (mm_addr)(mmdata +nchannels*MM_SIZEOF_MODCH);
	mix_channels = (mm_addr)(mmdata +nchannels*(MM_SIZEOF_MODCH+MM_SIZEOF_ACTCH));
	wavebuffer = (mm_addr)(mmdata +nchannels*(MM_SIZEOF_MODCH+MM_SIZEOF_ACTCH+MM_SIZEOF_MIXCH));
	
	mmsystem.mixing_mode = MM_MIX_16KHZ;
	mmsystem.mod_channel_count =nchannels;
	mmsystem.mix_channel_count =nchannels;
	mmsystem.module_channels = mod_channels;
	mmsystem.active_channels = act_channels;
	mmsystem.mixing_channels = mix_channels;
	mmsystem.mixing_memory = (mm_addr)mixbuffer;
	mmsystem.wave_memory = (mm_addr)wavebuffer;
	mmsystem.solution = (mm_addr)mmsolution_bin;
	
	mmInit( &mmsystem );
	mmInstallCallback( mmCallback );
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	
	commsInit();
	
	irqInit();
	irqSet( IRQ_VBLANK, mmVBlank );
	irqEnable( IRQ_VBLANK );
	
	// use this timer for delays!
	irqEnable( IRQ_TIMER3 );
	
	irqSet( IRQ_VCOUNT, maxmodUpdate );
	
	setupMaxmod();
	
	copyGraphics();
	
	// INITIALIZE GAME STUFF
	// collision...
	collisionInit();
	enemiesInit();
	planeSetup();
	bulletsInit();
	
	
	VBlankIntrWait();
	irqEnable( IRQ_VCOUNT );
	
	while( 1 )
	{
		beginIntro();
		{
	//		int delay1;
	//		for( delay1= 0; delay1 < DELAY_BEFORE_TITLE; delay1++ ) VBlankIntrWait();
		}
happyness:
		storyPlay( storyline_intro );
		
		int bt = beginTitle();
		{
			int delay1;
			for( delay1= 0; delay1 < DELAY_BEFORE_TITLE; delay1++ ) VBlankIntrWait();
		}
		
		if( bt==2 )
			goto happyness;
		
		resetGame();
		
		// show game start!
		showGamestart();
		
redo_level0:
		playLevel( 0 );
		if( game_over )
		{
			if( credits )
				ask_continue=1;
			storyPlay( storyline_d1 );
			if( ask_continue )
			{
				ask_continue=0;
				p1.score = 0;
				P1_LIVES = 2;
				goto redo_level0;
			}
			continue;
		}
		storyPlay( storyline_1 );
		
redo_level1:
		playLevel( 1 );
		if( game_over )
		{
			if( credits )
				ask_continue=1;
			storyPlay( storyline_d2 );
			if( ask_continue )
			{
				ask_continue=0;
				p1.score = 0;
				P1_LIVES = 2;
				goto redo_level1;
			}
			continue;
		}
		storyPlay( storyline_2 );
		
redo_level2:
		playLevel( 2 );
		
		if( game_over )
		{
			if( credits )
				ask_continue=1;
			storyPlay( storyline_d3 );
			if( ask_continue )
			{
				ask_continue=0;
				p1.score = 0;
				P1_LIVES = 2;
				goto redo_level2;
			}
			continue;
		}
		storyPlay( storyline_3 );
	}
}
