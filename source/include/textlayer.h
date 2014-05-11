#ifndef TEXTLAYER_H
#define TEXTLAYER_H

// init text system
void textInit( u8 screen_base, u8 char_base );

// calculate string length
int textCalcLength( char* string );

// draw text (slow)
void textDraw( int x, int y, int speed, char* string );

// draw text centered (slow)
void textDrawCenter( int y, int speed, char* string );
// clear text
void textClear( int x, int y, int length );
void textClearLine( int y);

// print a character from the string (for slow draw)
void textPrintCharacter( void );
// update text
void textUpdate( void );


void textDrawChar( int x, int y, char c );
// draw immediate text
void textDrawImm( int x, int y, char* string );
// and centered
void textDrawImmCenter( int y, char* string );
// phew

void textDrawNumber( int x, int y, int len, int number );

extern u8 text_write_active;

#endif
