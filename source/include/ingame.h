#ifndef INGAME_H
#define INGAME_H


#define fademode_end 3
#define fademode_in  2
#define fademode_out 1
#define fademode_none 0
extern int game_fade;
extern int game_fademode;

extern u8	stats_dirty_p1;
extern u8	stats_dirty_p2;
extern u8	game_over;

void resetGame( void );
void playLevel( int level );
void flashScreen( int flash_time );

#endif
