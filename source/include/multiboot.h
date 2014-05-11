#ifndef MULTIBOOT_H
#define MULTIBOOT_H

void commsInit( void );
u16 commsCheckConnection( void );
void commsMultiboot( void );

extern u8 comm_master;

#endif
