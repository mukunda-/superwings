#include <gba.h>
#include <gfx_font1.h>

#include <maxmod.h>
#include "mmsolution.h"

u16* text_characters;
u16* text_bg;

u16* text_write_pos;
u16 text_write_x;
u16 text_write_y;
int text_write_counter;
int text_write_speed;
u16 text_write_active;
char* text_write_string;

// init text system
void textInit( u8 screen_base, u8 char_base )
{
	REG_BG3CNT = BG_MAP_BASE( screen_base ) | TILE_BASE( char_base );
	text_characters = TILE_BASE_ADR( char_base );
	text_bg = MAP_BASE_ADR( screen_base );
	text_write_active=0;
	
	LZ77UnCompVram( (void*)gfx_font1Tiles, (void*)(text_characters) );
	
	int x;
	for( x = 0; x < 1024; x++ )
	{
		text_bg[x] = 0;
	}
	
}

// calculate string length
int textCalcLength( char* string )
{
	int x=0;
	int length=0;
	while( string[x++] != 0 ) length++;
	return length;
}

// draw text (slow)
void textDraw( int x, int y, int speed, char* string )
{
	text_write_x = x;
	text_write_y = y;
	text_write_pos = text_bg + x+y*32;
	text_write_counter = 0;
	text_write_speed = speed;
	text_write_active=1;
	text_write_string = string;
}

// draw text centered (slow)
void textDrawCenter( int y, int speed, char* string )
{
	int length = textCalcLength( string );
	textDraw( 15 - length/2, y, speed, string );
}

// clear text
void textClear( int x, int y, int length )
{
	while( length != 0 )
	{
		text_bg[x+y*32+length-1] = 0;
		length--;
	}
}

void textClearLine( int y )
{
	textClear( 0, y, 32 );
}

// print a character from the string (for slow draw)
void textPrintCharacter( void )
{
	if( text_write_active )
	{
		if( *text_write_string != 0 )
		{
			if( *text_write_string == 13 )
			{
				text_write_string++;
				text_write_pos = text_bg + text_write_x+text_write_y*32+32;
				text_write_y++;
				
			}
			else if( *text_write_string == 1 )
			{
				text_write_string++;
			}
			else
			{
				
				*text_write_pos++ = *text_write_string;
				if( (*text_write_string) != ' ' )
					mmEffect( SFX_BEEP1 );
				text_write_string++;
				
			}
		}
		else
		{
			text_write_active=0;
		}
	}
}

// update text
void textUpdate( void )
{
	if( text_write_active )
	{
		text_write_counter += text_write_speed;
		while( text_write_counter >= 256 )
		{
			text_write_counter -= 256;
			textPrintCharacter();
		}
	}
}

void textDrawChar( int x, int y, char c )
{
	text_bg[x+y*32] = c;
}

// draw immediate text
void textDrawImm( int x, int y, char* string )
{
	int l=0;
	int rx=x;
	while( string[l] != 0 )
	{
		if( string[l] == 13 )
		{
			x=rx;
			y++;
		}
		else
		{
			text_bg[x+y*32] = string[l];
			x++;
		}
		l++;
	}
}

// and centered
void textDrawImmCenter( int y, char* string )
{
	int length = textCalcLength( string );
	textDrawImm( 15-length/2, y, string );
}

void textDrawNumber( int x, int y, int len, int number )
{
	int c;
	while( len > 0 )
	{
		c = number & 0xF;
		number >>= 4;
		text_bg[x+y*32]=c+'0';
		x--;
		len--;
	}
}

// phew
