#include <gba.h>
#include "sprites.h"
#include "gfx_gamestart.h"

#define gs_delay 60*3
#define gs_delayout 60

#define bg_map ((u16*)0x6000000)

void showGamestart( void )
{
	LZ77UnCompVram( (void*)gfx_gamestartTiles, (void*)(0x6000800) );
	int x,y;
	for( x =0 ; x < 1024; x++ )
		bg_map[x] = 128;
	for( x = 0; x < 120/8; x++ )
		for( y = 0; y < 24/8; y++ )
			bg_map[x+y*32] = x+y*(120/8) + 64;
			
	for( x =0 ;x < 16; x++ )
		((u16*)(0x6001000))[x]=0;	// tile 96
		
	REG_BG0HOFS = -60;
	REG_BG0VOFS = -68;
	
	REG_BG0CNT = 0;
	
	VBlankIntrWait();
	SetMode( MODE_0 | BG0_ON );
	
	int f=gs_delay;
	while(f--) VBlankIntrWait();
	SetMode( 0 );
	
	REG_BG0HOFS=0;
	REG_BG0VOFS=0;
	
	f=gs_delayout;
	while(f--) VBlankIntrWait();
}
