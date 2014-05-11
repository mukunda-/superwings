#ifndef OEFIHPWOEIFGHAEGOPHAWEPOGAHIWEPOGIWHAEG
#define OEFIHPWOEIFGHAEGOPHAWEPOGAHIWEPOGIWHAEG

typedef struct __attribute__((aligned(4))) tplayerdata 
{
	u8 x;
	u8 y;
	u8 a;
	u8 flags;
	u8 ammo;	// 2 nibbles: missiels/bombs
	u8 reserved1;
	u16 checksum;
} playerdata;

// flags:
#define PDFLAG_FIRING	1
#define PDFLAG_HFLIP	2
#define PDFLAG_TURNING	4
#define PDFLAG_INVUN	8
#define PDFLAG_FIRINGM	16

void delayCycles( u16 amount );
void delayCycles64( u16 amount );
void commInterrupt(void);
void commBegin( void );
u16 commSend16( u16 value );
void commFullSyncRecv( void );
void commFullSync( void );

void commTransferData( void );

extern u8 comm_connection;
/*
void ext_keysScan();
int ext_keysDown();
int ext_keysHeld();
*/
#endif
