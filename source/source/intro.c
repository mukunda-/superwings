#include <gba.h>
#include "gfx_intro.h"
#include "main.h"

#define bgdata ((u16*)(0x6005000))

#define INTRO_TIMEOUT (60*3)	// (in frames)
#define FADEOUT_SPEED	70

#include "maxmod.h"
#include "mmsolution.h"

#define graytone(x) ((x)+((x)<<5)+((x)<<10))
#define hexcolor(h) (((((h)&255)>>3)<<10)+(((((h)>>8)&255)>>3)<<5)+(((((h)>>16)&255)>>3)))

const u16 intro_pal[4] = {graytone(0), graytone(12), graytone(22), graytone(31)};
//const u16 intro_pal[4] = {hexcolor(0x000000), hexcolor( 0x393939 ),hexcolor( 0x0133EA ), hexcolor( 0xFFFFFF ) };

void beginIntro( void )
{
	// copy graphics to vram
	LZ77UnCompVram( (void*)gfx_introTiles, (void*)0x6000000 );
	//dmaCopy(  (void*)gfx_introPal, (void*)0x5000000, gfx_introPalLen );
	//setPalette( palette_gray );
	setPalette( intro_pal );
	copyPalette( -4 );
	
	REG_BG0CNT = BG_MAP_BASE(0x5000/0x800);
	REG_BG0HOFS=0;
	REG_BG0VOFS=0;
	
	int x,y;
	for( x = 0; x < 30; x++ )
		for( y = 0; y < 20; y++ )
			bgdata[x+y*32] = x+y*30;
	
	//mmStart( MOD_JINGLE1, MM_PLAY_ONCE );
	VBlankIntrWait();
	
	SetMode(MODE_0 | BG0_ON | OBJ_1D_MAP);
	
	int timer = INTRO_TIMEOUT;
	int keysd;
	
	int fade=-4<<4;
	
	int draw_line;
	while(timer != 0) 
	{
		timer--;
		VBlankIntrWait();
		scanKeys();
		keysd=keysDown();
		if( (keysd & KEY_A) || (keysd & KEY_START) )
		{
			timer=0;
		}
		if( fade < 0 )
			fade+=4;
		copyPalette( fade>>4 );
	}
	
	while( fade > -4<<4 )
	{
		VBlankIntrWait();
		fade-=4;
		copyPalette( fade>>4 );
		
	}
	
	/*int fade =0;
	
	REG_BLDY = 0;
	REG_BLDCNT = 0xFF;
	
	
	while( fade < 16<<8 )
	{
		fade += FADEOUT_SPEED;
		VBlankIntrWait();
		REG_BLDY = fade>>8;
	}
	
	REG_BLDCNT=0;
	*/
	SetMode( 0 );
	
}
