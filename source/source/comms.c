//----------------------------------
// COMMUNICATION INTERFACE
//----------------------------------

// this whole thing is a nasty mess!

#define debug

//-----------------------------
#include <gba.h>
#include <stdlib.h>
#include "multiboot.h"
#include "comms.h"
#include "collision.h"
#include "enemies.h"
#include "bigplane.h"
#include "heli.h"
#include "lilplane.h" 
#include "tank.h"
#include "bullet.h"
#include "plane.h"
#include "level.h"

#define Q_ENTRIES 		64
#define Q_ENTRIES_MASK	63

#define CX_FULLSYNC	0x101a
#define CR_FULLSYNC	0x10fa

//-------------------------------------
// full sync stuff

// things

#define SDATA_BIGPLANE	ETYPE_BIGPLANE
#define SDATA_HELI		ETYPE_HELI
#define SDATA_LILPLANE	ETYPE_JET
#define SDATA_TANK		ETYPE_TANK

// other stuff
// 

#define SDATA_BULLETS	0x21
#define SDATA_PLANE1	0x22
#define SDATA_PLANE2	0x23
#define SDATA_OTHER		0x24

// terminate
#define SDATA_END		0x10EE

// transfer lengths
static const u8 sdata_sizes[] = { 0, sizeof( bigplane ), sizeof( helicopter ), sizeof( lilplane ), sizeof( tank ) };

//------------------------------------
// frame packets

#define FDATA_EXCHANGE	0x102a

#define FMODE_STANDBY	0x00
#define FMODE_EXCH		0x01
#define FMODE_NULL		0x02

#define FMODE_RECV_SKIP	0x02

//#define CMODE_NORMAL	0x3A

//------------------------------------
// SIOCNT definitions

#define SIO_READY	(1<<3)
#define mode_sb		(SIO_MULTI+SIO_IRQ+SIO_115200)
#define mode_tr		(SIO_32BIT+SIO_IRQ)

//-----------------------------------------------------------
// Wait for serial interrupt
//-----------------------------------------------------------
static inline void waitCereal()
//-----------------------------------------------------------
{
	IntrWait( 1, IRQ_SERIAL );
}

typedef struct
{
	u16	data[Q_ENTRIES];
	u16 read;
	u16 write;
} fifo;

// crap...
//playerdata 	pdata;
//u16		pd_read;

//u8		pd_reading;

int		pd_recv_mode;
int		pd_recv_times;
int		pd_send_mode;
int		pd_send_times;

fifo	fsend;
fifo	frecv;

int commMode;

u8 comm_connection=0;

void fifoPushData( fifo* f, u16* data, u16 words )
{
	int x;
	int ime=REG_IME;
	REG_IME=0;
	for( x = 0; x < words; x++ )
	{
		f->data[f->write] = data[x];
		f->write = (f->write + 1) & Q_ENTRIES_MASK;
	}
	REG_IME=ime;
}

void fifoPopData( fifo* f, u16* data, u16 words )
{
	int x;
	int ime=REG_IME;
	REG_IME=0;
	for( x = 0; (x < words) && (f->read != f->write); x++ )
	{
		data[x] = f->data[f->read];
		f->read = (f->read + 1) & Q_ENTRIES_MASK;
	}
	REG_IME=ime;
}

void fifoPush( fifo* f, u16 data )
{
	int ime=REG_IME;
	REG_IME=0;
	f->data[f->write] = data;
	f->write = (f->write + 1) & Q_ENTRIES_MASK;
	REG_IME=ime;
}

u16 fifoPop( fifo* f )
{
	u16 r;
	int ime=REG_IME;
	REG_IME=0;
	if( f->write == f->read )
	{
		r = 0;
	}
	else
	{
		r = f->data[f->read];
		f->read++;
		f->read &= Q_ENTRIES_MASK;
	}
	REG_IME=ime;
	return r;
}

u16 fifoSize( fifo* f )
{
	int r;
	r = f->write - f->read;
	return r < 0 ? (Q_ENTRIES+r) : r;
}

//--------------------------------------------------------
// Waste clock cycles
//--------------------------------------------------------
void delayCycles( u16 amount )
//--------------------------------------------------------
{
	REG_TM3CNT_L = -amount;
	REG_TM3CNT_H = TIMER_IRQ + TIMER_START;
#ifndef debug
	IntrWait( 1, IRQ_TIMER3 );
#else
	int a = REG_TM3CNT_L;
	while( REG_TM3CNT_L >= a )
		a = REG_TM3CNT_L;
#endif
	REG_TM3CNT_H = 0;
}
/*
//--------------------------------------------------------
// Waste more clock cycles (amount*64)
//--------------------------------------------------------
void delayCycles64( u16 amount )
//--------------------------------------------------------
{
	REG_TM3CNT_L = -amount;
	REG_TM3CNT_H = TIMER_IRQ + TIMER_START + 1;
	IntrWait( 1, IRQ_TIMER3 );
	REG_TM3CNT_H = 0;
}*/

/*
//--------------------------------------------------------
void commSendIRQ( void )
//--------------------------------------------------------
{
	REG_SIOMLT_SEND = REG_KEYINPUT;
	REG_TM3CNT_H=0;
	REG_SIOCNT |= SIO_START;
}*/

//--------------------------------------------------------
// this function is for the slave only
// the master does the transfers in the frame routine
//--------------------------------------------------------
void commInterrupt(void)
//--------------------------------------------------------
{
	// read incoming data
	u16 read = REG_SIOMULTI0;
	
	switch( pd_recv_mode )
	{
	case FMODE_STANDBY:
		
		switch( read )
		{
		
		case FDATA_EXCHANGE:
			pd_recv_mode = FMODE_EXCH;
			pd_recv_times = (sizeof( playerdata ) >> 1);
			if( fifoSize( &fsend ) >= (sizeof( playerdata ) >> 1) )
			{
				pd_send_mode = FMODE_EXCH;
				pd_send_times = (sizeof( playerdata ) >> 1);
			}
			else
			{
				pd_send_mode = FMODE_NULL;
				pd_send_times = (sizeof( playerdata ) >> 1);
			}
			break;
		}
		break;
		
	case FMODE_EXCH:
		fifoPush( &frecv, read );
		pd_recv_times--;
		if( pd_recv_times == 0 )
			pd_recv_mode = FMODE_STANDBY;
		break;
	}
	
	switch( pd_send_mode )
	{
	case FMODE_EXCH:
		REG_SIOMLT_SEND = fifoPop( &fsend );
		break;
		
	case FMODE_NULL:
		REG_SIOMLT_SEND = 0xFFFF;
		break;
	}
	
	if( pd_send_mode )
	{
		pd_send_times--;
		if( pd_send_times == 0 )
			pd_send_mode = FMODE_STANDBY;
	}
}

//------------------------------------------------------
// full sync and start communication
//------------------------------------------------------
void commBegin( void )
//------------------------------------------------------
{
	fsend.read = fsend.write = 0;
	frecv.read = frecv.write = 0;
	comm_connection=1;

	if( comm_master )
	{
		commMode=0;
		REG_SIOCNT = mode_sb;
		irqEnable( IRQ_SERIAL );
		commFullSync();
	}
	else
	{
		commMode=0;
		REG_SIOCNT = mode_sb;
		irqEnable( IRQ_SERIAL );
		commFullSyncRecv();
		irqSet( IRQ_SERIAL, commInterrupt );
	}
}

//------------------------------------------------------
// frame routine
//------------------------------------------------------
void commTransferData( void )
//------------------------------------------------------
{
	if( comm_connection )
	{
		if( comm_master )
		{
			int x;
			playerdata pdata;
			
			// build player data
			planeMakeData( &p1, &pdata );
			fifoPushData( &fsend, (u16*)&pdata, sizeof(playerdata)>>1 );
			
			// send exchange signal
			REG_SIOMLT_SEND = FDATA_EXCHANGE;
			REG_SIOCNT |= SIO_START;
			while( REG_SIOCNT & SIO_START ) {}
			
			// exchange player data
			for( x = 0; x < sizeof( playerdata) /2; x++ )
			{
				// queue player data
				REG_SIOMLT_SEND = fifoPop( &fsend );
				
				// delay some cycles
				//asm( "mov r11,r11" );
				delayCycles( 1000 );
				
				// transfer data
				REG_SIOCNT |= SIO_START;
				while( REG_SIOCNT & SIO_START ) {}
				
				// write received data
				fifoPush( &frecv, REG_SIOMULTI1 );
			}
			fifoPopData( &frecv, (u16*)&pdata, (sizeof( playerdata ) >> 1) );
			planeReadData( &p2, &pdata );
		}
		else
		{
			// this is a bit nasty
			playerdata pdata;
			planeMakeData( &p2, &pdata );
			fifoPushData( &fsend, (u16*)&pdata, sizeof( playerdata ) >> 1 );
			if( fifoSize( &frecv ) >= (sizeof( playerdata ) >> 1) )
			{
				fifoPopData( &frecv, (u16*)&pdata, (sizeof( playerdata ) >> 1) );
				planeReadData( &p1, &pdata );
			}
		}
	}
}

//=================================================================
//=================================================================
//                            FULL SYNC
//=================================================================
//=================================================================


u16 commSend16( u16 value )
{
	// MASTER ONLY
	while( !(REG_SIOCNT & SIO_READY) ) {       }
	REG_SIOMLT_SEND = value;
	REG_SIOCNT |= SIO_START;
	waitCereal();
	delayCycles( 1000 );//600 );
	return REG_SIOMULTI1;
}

void commsFillBuffer( u16* buffer, int len )
{
	u16 data;
	u32 write=0;
	while( len > 0 )
	{
		waitCereal();
		data = REG_SIOMULTI0;
		buffer[write++] = data;
		len -= 2;
	}
}

//--------------------------------------------------------
// Send buffer... :|
//--------------------------------------------------------
void commsSendBuffer( u16* buffer, int len )
//--------------------------------------------------------
{
	u32 read=0;
	while( len > 0 )
	{
		commSend16( buffer[read++] );
		len -= 2;
	}
}

//--------------------------------------------------------
// Read enemy data
//--------------------------------------------------------
void commsRead_enemy( int type )
//--------------------------------------------------------
{
	int size = sdata_sizes[type];
	void* buffer = malloc( size );
	commsFillBuffer( buffer, size );
	enemyRegister( (enemy*)buffer, type );
}

//--------------------------------------------------------
// Send enemy data
//--------------------------------------------------------
void commsWrite_enemy( enemy* enemy )
//--------------------------------------------------------
{
	int size = sdata_sizes[ enemy->type ];
	
	commSend16( enemy->type );
	commsSendBuffer( (void*)enemy, size );
}

//--------------------------------------------------------
// Read bullet data
//--------------------------------------------------------
void commsRead_bullets( void )
//--------------------------------------------------------
{
	commsFillBuffer( ((u16*)bullets), MAX_BULLETS * sizeof( bullet ) );
}

void commFullSyncRecv( void )
{
	u32 buffer_wr;
	u32 data;
	enemiesCleanup();
	
	do
	{
		waitCereal();
		data = REG_SIOMULTI0;
	} while( data != CX_FULLSYNC );
	
	REG_SIOMLT_SEND = CR_FULLSYNC;
	
	do
	{
		waitCereal();
		REG_SIOMLT_SEND=0;
		data = REG_SIOMULTI0;
		buffer_wr = 0;
		// receive data
		if( data < 10 )
		{
			commsRead_enemy( data );
		}
		else
		{
			switch( data )
			{
			case SDATA_BULLETS:
				commsRead_bullets();
				break;
			case SDATA_PLANE1:
				commsFillBuffer( (u16*)&p1, sizeof( plane ) - sizeof( collbox ) );
				break;
			case SDATA_PLANE2:
				commsFillBuffer( (u16*)&p2, sizeof( plane ) - sizeof( collbox ) );
				break;
			case SDATA_OTHER:
				waitCereal(); data = REG_SIOMULTI0;
				waitCereal(); data |= REG_SIOMULTI0<<16;
				level_pos = data;
				waitCereal(); data = REG_SIOMULTI0;
				waitCereal(); data |= REG_SIOMULTI0<<16;
				scroll_pos=data;
				waitCereal(); data = REG_SIOMULTI0;
				waitCereal(); data |= REG_SIOMULTI0<<16;
				level_scroll_speed=data;
				waitCereal(); data = REG_SIOMULTI0;
				waitCereal(); data |= REG_SIOMULTI0<<16;
				script_pos = data;
				waitCereal(); data = REG_SIOMULTI0;
				waitCereal(); data |= REG_SIOMULTI0<<16;
				delta_counter = data;
				
				levelPatch();
				break;
				
			default:
				
				break;
			}
		}
		
		REG_SIOMLT_SEND = CR_FULLSYNC;
	} while( data != SDATA_END );
}

void commFullSync( void )
{
	irqSet( IRQ_SERIAL, 0 );
	
	while( commSend16( CX_FULLSYNC ) != CR_FULLSYNC ) {}
	
	enemy* e;
	for( e = enemy_first; e; e = e->next )
	{
		commsWrite_enemy( e );
		while( commSend16( CX_FULLSYNC ) != CR_FULLSYNC ) {}
	}
	
	// send bullets... etc
	commSend16( SDATA_BULLETS );
	commsSendBuffer( ((u16*)bullets), MAX_BULLETS * sizeof(bullet) );
	
		while( commSend16( CX_FULLSYNC ) != CR_FULLSYNC ) {}
	
	// send plane info
	commSend16( SDATA_PLANE1 );
	commsSendBuffer( ((u16*)&p1), sizeof(plane) - sizeof( collbox ));
	
		while( commSend16( CX_FULLSYNC ) != CR_FULLSYNC ) {}
	
//	commSend16( SDATA_PLANE2 );
//	commsSendBuffer( ((u16*)&p2), sizeof(plane) - sizeof( collbox ) );
	
//		while( commSend16( CX_FULLSYNC ) != CR_FULLSYNC ) {}
		
	commSend16( SDATA_OTHER );
	commSend16( level_pos&0xFFFF );
	commSend16( level_pos>>16 );
	commSend16( scroll_pos&0xFFFF );
	commSend16( scroll_pos>>16 );
	commSend16( level_scroll_speed&0xFFFF );
	commSend16( level_scroll_speed>>16 );
	commSend16( script_pos&0xFFFF );
	commSend16( script_pos>>16 );
	commSend16( delta_counter&0xFFFF );
	commSend16( delta_counter>>16 );
	
		while( commSend16( CX_FULLSYNC ) != CR_FULLSYNC ) {}
	
	commSend16( SDATA_END );
}
