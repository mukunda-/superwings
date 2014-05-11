#ifndef SPRITES_H
#define SPRITES_H

typedef struct tsprite
{
	u16	attr0;
	u16	attr1;
	u16	attr2;
} sprite;

#define SPRITE_SIZE_8	0
#define SPRITE_SIZE_16	1
#define SPRITE_SIZE_32	2
#define SPRITE_SIZE_64	3

/*
void spritesClearAll( void );
void spriteSet( int index, int x, int y, int tile, int shape, int size );
void spritePosition( int index, int x, int y );
void spritePositionX( int index, int x );

void spriteFlipX( int index );
void spriteFlipY( int index );

void spriteDisable( int index );
void spriteEnableScaling( int index, int affindex );
void spriteEnableScalingD( int index, int affindex );
void spriteSetAffineMatrix( int index, int xx, int xy, int yx, int yy );

void spritesBegin( int index );
void spritesEnd( void );

int spriteAdd8( int x, int y, int tile );
int spriteAdd16( int x, int y, int tile );
int spriteAdd32( int x, int y, int tile );
int spriteAdd64( int x, int y, int tile );
int spriteAdd8x16( int x, int y, int tile );
int spriteAdd16x8( int x, int y, int tile );

void spritesSetupAffine( void );

void spriteSetPriority( int index, int prio );
void spriteAddPalette( int index, int palette );*/

void spritesEraseOAM( void );
void spritesSetupAffine( void );

void spritesBegin( void );
void spritesEnd( void );

sprite*	spriteAdd( int x, int y, int tile, int size, int priority );

void spriteWide( sprite* s );
void spriteTall( sprite* s );
void spriteFlipX( sprite* s );
void spriteFlipY( sprite* s );
void spriteSetAffineMatrix( int index, int xx, int xy, int yx, int yy );
void spriteEnableRotation( sprite* s, int angle, int doublesize );
void spriteAddPalette( sprite* s, int palette );




#endif
