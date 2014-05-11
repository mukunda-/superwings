//--------------------------------------------
// sprite handler
//--------------------------------------------

#include <gba.h>
#include "tables.h"
#include "sprites.h"

// read priority bits
#define PRIO_MASK( x ) ( ((x) >> 10) & 3 )

//--------------------------------------------
// sprite buffer
//--------------------------------------------
sprite	sprites[128];

//--------------------------------------------
// and sprite counter
//--------------------------------------------
int	new_sprite;

//--------------------------------------------
// Initialize OAM
//--------------------------------------------
void spritesEraseOAM( void )
{
	int x;
	for( x= 0 ; x < 128; x++ )
		OAM[x].attr0 = ATTR0_DISABLED;
}

//--------------------------------------------
// Toggle horizontal flip
//--------------------------------------------
void spriteFlipX( sprite* s )
{
	if( s )
		s->attr1 ^= ATTR1_FLIP_X;
}

//--------------------------------------------
// Toggle vertical flip
//--------------------------------------------
void spriteFlipY( sprite* s )
{
	if( s )
		s->attr1 ^= ATTR1_FLIP_Y;
}

//--------------------------------------------
// Enable rotation, angle = 0->15
//--------------------------------------------
void spriteEnableRotation( sprite* s, int angle, int doublesize )
{
	if( s )
	{
		s->attr0 |= OBJ_ROT_SCALE_ON | (doublesize ? ATTR0_ROTSCALE_DOUBLE : 0);
		s->attr1 |= OBJ_ROT_SCALE(angle);
	}
}

//--------------------------------------------
// Setup affine matrix
//--------------------------------------------
void spriteSetAffineMatrix( int index, int xx, int xy, int yx, int yy )
{
	// dummy :P
	OAM[(index<<2)+0].dummy = xx;
	OAM[(index<<2)+1].dummy = xy;
	OAM[(index<<2)+2].dummy = yx;
	OAM[(index<<2)+3].dummy = yy;
}

//--------------------------------------------
// Begin sprite buffering
//--------------------------------------------
void spritesBegin( void )
{
	new_sprite = 0;
}

//--------------------------------------------
// End sprite buffering, copy to OAM
//--------------------------------------------
void spritesEnd( void )
{
	int s;
	int obj;
	int prio;
	
	obj = 0;
	
	// sort priorities and copy to OAM
	for( prio = 0; prio < 4; prio++ )
	{
		for( s = 0; s < new_sprite; s++ )
		{
			if( PRIO_MASK( sprites[s].attr2 ) == prio )
			{
				OAM[obj].attr0 = sprites[s].attr0;
				OAM[obj].attr1 = sprites[s].attr1;
				OAM[obj].attr2 = sprites[s].attr2;
				obj++;
			}
		}
	}
		
	// fill the rest of OAM with disabled sprites
	for(; obj < 128; obj++ )
		OAM[obj].attr0 = ATTR0_DISABLED;
}

//----------------------------------------------------------------------------
// Add a sprite
//----------------------------------------------------------------------------
sprite*	spriteAdd( int x, int y, int tile, int size, int priority )
{
	sprite* s = 0;
	if( new_sprite < 128 )
	{
		s = &sprites[new_sprite];
		s->attr0 = (y & 255);
		s->attr1 = (x & 511) + OBJ_SIZE(size);
		s->attr2 = tile + ATTR2_PRIORITY( priority );
		new_sprite++;
	}
	return s;
}
//----------------------------------------------------------------------------
// Widen sprite
//----------------------------------------------------------------------------
void spriteWide( sprite* s )
{
	if( s )
		s->attr0 += ATTR0_WIDE;
}

//----------------------------------------------------------------------------
// Lengthen sprite
//----------------------------------------------------------------------------
void spriteTall( sprite* s )
{
	if( s )
		s->attr0 += ATTR0_TALL;
}

//----------------------------------------------------------------------------
// Add palette (use only once)
//----------------------------------------------------------------------------
void spriteAddPalette( sprite* s, int palette )
{
	if( s )
		s->attr2 += ATTR2_PALETTE(palette);
}

//----------------------------------------------------------------------------
// Setup rotation matrices, and a couple size ones
//----------------------------------------------------------------------------
void spritesSetupAffine( void )
{
	// make 16 rotation angles
	int x;
	for( x = 0; x < 16; x++ )
		spriteSetAffineMatrix( x, (sin_tab[((x*16)+64) & 255]+32)>>6, (sin_tab[x*16]+32)>>6, -((sin_tab[x*16]+32)>>6), (sin_tab[((x*16)+64) & 255]+32)>>6 );
		
	spriteSetAffineMatrix( 16, 128, 0, 0, 128 );	// double size
	spriteSetAffineMatrix( 17, 192, 0, 0, 192 );	// small-large
	spriteSetAffineMatrix( 18, 300, 0, 0, 300 );	// small
}
