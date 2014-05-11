@ speedloader :)

.section ".boot", "ax", %progbits

.equ	LZ77UnpackVram, 0x120000
.equ	CpuSet, 0x0B0000
.equ	BitUnPack, 0x10

@============================
@ multiboot header
@============================

b	_start
.byte	0,0	@ something, boot method
.hword	0xf00d

@============================
@ unpack program
@============================

.global	_start

_start:

@ unpack loader image into iwram
	
	adr	r0, image_start_heh
	mov	r1, #0x5000000
	add	r1, #0x200
	add	r7, r1, #9	@ r7 = program address (skip data)
	swi	LZ77UnpackVram

@ jump to loader

	bx	r7

image_start_heh:

@============================
@ transfer routine
@============================

.section ".loader", "ax", %progbits
.thumb

.equ	RCNT, 		0x14
.equ	SIOCNT, 	0x08
.equ	SIODATA, 	0x00

.equ	BG0CNT,		0x08
.equ	BG2CNT,		0x0C
.equ	IE,		0x200
.equ	IF,		0x202
.equ	IME,		0x208

.equ	SIO_32BIT, 0b0001000000000000
@		        ^-32bit
.equ	SIO_IRQ,   0b0100000000000000
@		      ^-irq enable

.equ	INT_SERIAL, (1<<7)
.equ	SIO_START, (1<<7)

#define	REGS	r5
#define	ADDR	r6
#define	OAM	r7

.equ	IntrWait, 0x04

palette:
	.word	0x01970000,0x75a5029e
	
@------------------------------------------------
main:
@------------------------------------------------

@ regs...
	ldr	REGS,=0x4000000-4
	ldr	r0,=pirateTiles
	ldr	r1,=0x6010000+32
	adr	r2, unpackdata
	swi	BitUnPack
	
	mov	OAM, #0x07
	lsl	OAM, #24
	ldr	r0,=144 + (1<<(14+16))
	str	r0, [OAM]

@ install interrupt
 
	mov	r2, #0b1000001	@ sprites enabled, 1D
	lsl	r2, #6
	adr	r1, interrupt
	stmia	REGS!, {r1,r2}	@ <-- irq vector & DISPCNT
	add	REGS, #(0x120-4)/2
	add	REGS, #(0x120-4)/2
	
	mov	r1, #INT_SERIAL		@ used belooow!
	mov	r0, #IE-0x120
	strh	r1, [REGS, r0]
	mov	r2, #IME-0x120
	mov	r0, #1			@ used below, below, and below!!
	str	r0, [REGS, r2]
	
@ setup comms
@ RCNT is already initialized	

	mov	r3, #SIO_32BIT>>8	@ switch transfer mode
	lsl	r3, #8
	strh	r3, [REGS, #SIOCNT]

@ master side should detect SD switch from HIGH to LOW
@ (multiplayer mode outputs SD high during inactivity, normal mode doesn't)
@ (we are still in multiplayer mode from multibooting)

@ set transfer address

	ldr	ADDR,=0x20000C0
	
@ transfer image

	ldr	r4,=SIO_32BIT+SIO_IRQ+SIO_START
	strh	r4, [REGS, #SIOCNT]
	
	swi	IntrWait	@ r0,r1 are set above!
	
@ start program

	mov	r0, #0x2
	lsl	r0, #24
	add	REGS, #IE-0x120
	strh	r0, [REGS]	@ CLEAR IE
	strh	r0, [REGS,#8]	@ CLEAR IME
	add	r0, #0xc0
	mov	r1, #3		@ set boot method
	strb	r1, [r0, #4]
	
@ jump to program

	bx	r0

.align
unpackdata:
	.hword	(32*16)/4
	.byte	2
	.byte	4
	.word	0

.arm

@------------------------------------------------------------------
interrupt:
@------------------------------------------------------------------

	add	r0, pc, #1
	bx	r0
.thumb
	
@ read data

	ldr	r2, [REGS, #SIODATA]

@ set start bit

	ldr	r1,=SIO_32BIT+SIO_IRQ+SIO_START
	strh	r1, [REGS, #SIOCNT]
	
@ set IF flag

	mov	r1, #INT_SERIAL
	mov	r0, #IF-0x120
	strh	r1, [REGS, r0]
	
@ write data to memory

	stmia	ADDR!, {r2}
	lsr	r0, ADDR, #19
	bcc	1f

@ if zero, terminate IntrWait
	
	mov	r2, REGS
	sub	r2, #0x128/2
	sub	r2, #0x128/2
	strh	r1, [r2]	@ set bios irq flags
	
	
@ return to bios handler

1:	
	lsr	r0, ADDR, #10
	lsl	r0, #18
	lsr	r0, #2
	ldr	r1,=144 + (1<<(14+16))
	orr	r1, r0
	str	r1, [OAM]
	lsr	r1, #16+1
	mov	r0, #0b0100
	and	r1, r0
	add	r1, #1
	strh	r1, [OAM, #4]
	
	bx	lr

.pool
