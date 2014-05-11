// nocash routines

#ifndef NOCASH_H
#define NOCASH_H

// Install Burstboot Backdoor
void nocashInstallBurstboot( void );

// Print Message
void nocashPrint( char* ) __attribute((long_call));

#endif
