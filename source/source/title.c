#include <gba.h>
#include "sprites.h"
#include "gfx_superwangs.h"
#include "textlayer.h"
#include "level1.map.h"
#include "level.h"
#include "main.h"

#include <maxmod.h>
#include "mmsolution.h"

#define textmap ((u16*)0x6006800)

#define fadein_spd	100 //50

#define start_flash_speed 60*1

void copyBG( u16* src, u16* dest, u16 orr )
{
	u32* src32=(u32*)src;
	u32* dst32=(u32*)dest;
	u32  orr32;
	orr32 = orr | (orr<<16);
	u32 ecx;
	for( ecx = 256; ecx != 0; ecx-- )
	{
		*dst32 = (*src32) | orr32;
		dst32++;
		src32++;
		*dst32 = (*src32) | orr32;
		dst32++;
		src32++;
	}
}

int beginTitle( void )
{
	// copy graphics to vram
//	LZ77UnCompVram( (void*)gfx_titlebgTiles, (void*)0x6000000 );
//	dmaCopy(  (void*)gfx_titlebgPal, (void*)0x5000000, gfx_titlebgPalLen );
//	LZ77UnCompVram( (void*)gfx_logoTiles, (void*)logodata );
	
	textInit( 0xd000/0x800, 3 );
	
	levelInit();
	levelLoad( (void*)level1_mapTiles, (u16*)level1_mapPal, 206, (void*)level1_mapMap, 0,0 );//93, (void*)testmapMap, 0 );
	levelSwitchMode( 1 ); 
	LZ77UnCompVram( (void*)gfx_superwangsTiles, (void*)0x6004000 );
	
	int x,y;
	for( x = 0; x < 1024; x++ )
		textmap[x] = 0;
	for( x = 0; x < 27; x++ )
		for( y = 0; y < 10; y++ )
			textmap[2+x+(y+2)*32] = 512+x+y*27;
	
	for( x = 0; x < 27; x++ )
		textmap[9+x+(19*32)] = 512+270+x;
	REG_BG3CNT = SCREEN_BASE( 0x6800/0x800 );
	REG_BG3HOFS = 0;
	REG_BG3VOFS = 0;
	REG_BG1CNT = SCREEN_BASE( 0xd000/0x800 ) | CHAR_BASE( 3);
	REG_BG1HOFS=4;
	REG_BG1VOFS=4;
	REG_BG2HOFS=3;
	REG_BG2VOFS=3;
	//REG_BG2HOFS=-1;
	//REG_BG2VOFS=-1;
	REG_BG2CNT = SCREEN_BASE( 0xd800/0x800 ) | CHAR_BASE( 3);
	
	int fade=16<<8;
//	REG_BLDY = 16;
//	REG_BLDCNT = 0xFF;
	
	textDrawImmCenter( 14, "PRESS START" );
	copyBG( (void*)0x600d000, (void*)0x600d800, (4<<12) );
	
//	LZ77UnCompVram( (void*)gfx_startTiles, (void*)0x6010000 );
//	dmaCopy(  (void*)gfx_startPal, (void*)0x5000200, gfx_startPalLen );
	
	//spritesClearAll();
	
	// start image
	//spriteSet( 0, 88, 64, 0, ATTR0_WIDE, ATTR1_SIZE_64 );
	
	VBlankIntrWait();
	
	SetMode(MODE_0 | BG0_ON | BG3_ON | OBJ_1D_MAP);
	
//	screen_fadein( gfx_titlebgPal, 50 );
	
	//while(1) VBlankIntrWait();
	// fade in
	while( fade > (1<<9) )
	{
		levelUpdate();
		VBlankIntrWait();
		levelDraw();
		copyPalette( 0 + (fade>>10) );
		//REG_BLDY = fade>>8;
		fade -= fadein_spd;
		
	}
	
	REG_BLDCNT=0;
	REG_BLDY=0;
	
	int flashything=start_flash_speed;
	int keysd=0;
	
	int timeout = 10*60;
	
	while( !(keysd & KEY_START) ) 
	{	
		levelUpdate();
		VBlankIntrWait();
		levelDraw();
		flashything--;
		
		if( flashything == 0 )
		{
			flashything = start_flash_speed;
			REG_DISPCNT ^= BG1_ON + BG2_ON;
		}
		scanKeys();
		keysd = keysDown();
		timeout--;
		if ( timeout <= 0 )
		{
			//BG_PALETTE[0]=0;
			SetMode( 0 );
			return 2;
		}
	}
	
	mmStartSub( MOD_JCLICK );
	REG_DISPCNT |= BG1_ON + BG2_ON;
	textDrawImmCenter( 14, "             " );
	textDrawImm( 8, 15, "  START GAME" );
	//textDrawImm( 8, 16, "  OPTIONS" );
	copyBG( (void*)0x600d000, (void*)0x600d800, (4<<12) );
	textDrawChar( 8, 15, 0x03 );
	
	keysd=0;
	int sel=0;
	while( (!(keysd & KEY_START)) && (!(keysd&KEY_A)) ) 
	{	
		/*if( keysd & KEY_UP )
		{
			// beep
			mmStartSub( MOD_JSWITCH );
			sel=0;
			textDrawChar( 8, 15, 0x03 );
			textDrawChar( 8, 16,  0);
		}
		if( keysd & KEY_DOWN )
		{
			// beep
			mmStartSub( MOD_JSWITCH );
			sel=1;
			textDrawChar( 8, 15, 0 );
			textDrawChar( 8, 16, 0x03 );
		}*/
		levelUpdate();
		VBlankIntrWait();
		levelDraw();
		scanKeys();
		keysd = keysDown();
	}
	mmStartSub( MOD_JCLICK );
	
	SetMode( 0 );
	return sel;
}
