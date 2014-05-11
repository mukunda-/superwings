#ifndef LEVEL_H
#define LEVEL_H

void levelInit( void );

void levelClear( void );

void levelPatch( void );

void levelLoad( void* tiles, u16* palette, int y_size, void* map, void* script, int pos );
void levelUpdate( );
void levelDraw(void);
void levelSwitchMode( int demo );
void levelEnd( void );

extern u8* levelScript;
extern int level_scroll_speed;
extern int level_pos;
extern int scroll_pos;
extern int delta_counter;
extern int script_pos;

#define OBJCMD_NULL		0x00
#define OBJCMD_MOVEXY	0x01
#define OBJCMD_MOVEXA	0x0A
#define OBJCMD_MOVEYA	0x0B
#define OBJCMD_MOVEXYA	0x0C
#define OBJCMD_MOVE		0x02
#define OBJCMD_IDLE		0x03
#define OBJCMD_FIREP	0x04
#define OBJCMD_FIREA	0x05
#define OBJCMD_FIREM	0x06
#define OBJCMD_AIMP		0x07
#define OBJCMD_AIMA		0x08
#define OBJCMD_AIMM		0x09
#define OBJCMD_HALT		0x0D
#define OBJCMD_REMOVE	0x0E
#define OBJCMD_ITEM		0x0F

#define TEXT_START_X	3
#define TEXT_START_Y	9
#define TEXT_SPEED 50

#endif
