@--------------------------------
@ communication functions :)
@--------------------------------

@ no$gba does instant multiboot
@ our special uploader will only slow it down
@ also the uploader doesn't work in no$ :(
@#define nocash

.global commsInit
.global commsCheckConnection
.global commsMultiboot

.global comm_master

@----------------------------------------------------

.equ	SIOCNT_CTRL,	0x2003+(1<<14) @1001

.equ	RCNT, 0x4000134
.equ	SIOCNT, 0x4000128

.equ	SIOMLT_SEND, 0x400012A	@ Data Send Register (R/W)

.equ	SIOMULTI0, 0x4000120	@ SIO Multi-Player Data 0 (Parent) (R/W)
.equ	SIOMULTI1, 0x4000122	@ SIO Multi-Player Data 1 (1st child) (R/W)
.equ	SIOMULTI2, 0x4000124	@ SIO Multi-Player Data 2 (2nd child) (R/W)
.equ	SIOMULTI3, 0x4000126	@ SIO Multi-Player Data 3 (3rd child) (R/W)

.equ	SIO_START, (1<<7)

.equ	pal_dir, 0	@ 0..1	- ???
.equ	pal_color, 4	@ 0..6	- orange/red
.equ	pal_speed, 3	@ 0..3	- fast

.equ	PALETTE_DATA, (0x81+pal_color*0x10+pal_dir*8+pal_speed*2)

@----------------------------------------------------

.bss

comm_master:	.space	1

@----------------------------------------------------

.text
.thumb

.align 2
.thumb_func
@-------------------------------------------------------
commsInit:
@-------------------------------------------------------

@ switch to normal 16-bit multiplayer mode

	ldrb	r0,=__boot_method	@ check boot method
	ldrb	r0, [r0]		@ if 3 (multiplay boot), then
	cmp	r0, #3			@ this is a slave
	beq	2f			@ otherwise (cart/normal [xboo]) this is the master
	mov	r0, #1
	b	1f
2:	mov	r0, #0
1:	ldr	r1,=comm_master
	strb	r0, [r1]
	
	ldr	r1,=RCNT
	ldrh	r0, [r1]
	lsl	r0, #32-4
	lsr	r0, #32-4
	strh	r0 ,[r1]
	ldr	r0,=SIOCNT_CTRL
	ldr	r1,=SIOCNT
	strh	r0, [r1]
	
	bx	lr

.thumb_func
@-------------------------------------------------------
commsCheckConnection:
@-------------------------------------------------------
	
	ldr	r0,=comm_master
	ldrb	r0, [r0]
	cmp	r0, #1
	bne	.badconnection

	ldr	r0,=0x6200		@ multiboot detection
	ldr	r1,=SIOMLT_SEND
	strh	r0, [r1]
	ldr	r0,=SIOCNT
	ldrh	r1, [r0]
	lsr	r2, r1, #4		@ sd terminal
	bcc	.badconnection		@ 1 == all gba's ready
	lsr	r2, r1, #3		@ si terminal
	bcs	.badconnection		@ 0 == parent ( should be 0 )
	ldr	r1,=SIOCNT_CTRL|SIO_START
	strh	r1, [r0]		@ transfer hword

@ wait for value
1:	ldrh	r1, [r0]
	lsr	r1, #8
	bcs	1b
	
	ldr	r0,=SIOMULTI1		@ read return value
	ldrh	r0, [r0]		@ 0xFFFF = not ready
	bx	lr			@ 0x0000 = not ready
					@ 0x7202 = READY FOR TRANSMISSION
.badconnection:		@ invalid connection
	mov	r0,#0	@ return 0
	bx	lr	@
.pool

.thumb_func
@-------------------------------------------------------
commsMultiboot:
@-------------------------------------------------------

	push	{r4,r5,r6,lr}

	sub	sp, #0x4C
	mov	r0, sp
	mov	r1, #0xFF
	strb	r1, [r0, #0x1A]	@ client data 2,3
	strb	r1, [r0, #0x1B]
	mov	r1, #PALETTE_DATA
	strb	r1, [r0, #0x1C]
	mov	r1, #2
	strb	r1, [r0, #0x1E]	@ client bit
	add	r0, #0x20
#ifdef nocash
	ldr	r1,=0x2000000+0xC0
	str	r1, [r0]
	ldr	r1,=0x2000000+262144
	str	r1, [r0, #4]
#else
	ldr	r1,=speedloader_img
	str	r1, [r0]
	ldr	r1,=speedloader_img+0x150
	str	r1, [r0, #4]

#endif

	ldr	r3,=SIOCNT	@ get sio registers
	ldr	r2,=SIOMLT_SEND
	
@-------------------------------------------------------
.cm_restart:
@-------------------------------------------------------
	bl	commsDelay	@ DELAY 1/16s
	ldr	r4,=0x7202
				@ times	send	recv
	ldr	r0,=0x6200	@ skip	6200	0000
	bl	commsSendI	
	cmp	r0, #0
	beq	1f
	cmp	r0, r4
	bne	.cm_restart		
1:
	mov	r5, #15		@ 15	6200	7202	<- bit 1 set for slave 1
2:	ldr	r0,=0x6200
	bl	commsSendI
	cmp	r0, r4
	bne	.cm_restart
	sub	r5, #1
	bne	2b

	
	ldr	r0,=0x6102	@ 1	6102	7202
	bl	commsSendI
	cmp	r0, r4
	bne	.cm_restart
	
	ldr	r4,=0x2000000	@ 60h	xxxx	NN0x     Transfer C0h bytes header data in units of 16bits
	mov	r5, #0xC0
2:	ldrh	r0, [r4]
	add	r4, #2
	bl	commsSendI
	sub	r5, #2
	bne	2b


	ldr	r0,=0x6200	@ 1	6200	000x     Transfer of header data completed
	bl	commsSendI
	cmp	r0, #2
	bne	.cm_restart
	ldr	r0,=0x6202	@ 1	620y	720x     Exchange master/slave info again
	bl	commsSendI

	ldr	r0,=0x6300+PALETTE_DATA
	bl	commsSendI

	ldr	r4,=0x7202	@ 1	63pp	73cc     Send palette_data and receive client_data[1-3]
1:	ldr	r0,=0x6300+PALETTE_DATA
	bl	commsSendI
	cmp	r0, r4
	beq	1b		@ if data is 720x then retry

	ldr	r0,=0x6300+PALETTE_DATA
	bl	commsSendI

	mov	r5, sp
	strb	r0, [r5, #0x19]	@ client data

	add	r0, #0x11
	sub	r0, #2		@ other two clients are 0FFh
	
	lsl	r0, #32-8
	lsr	r0, #32-8
	
	strb	r0, [r5, #0x14]	@ handshake
	ldr	r1,=0x6400
	orr	r0, r1		@ 1	64hh	73uu	Send handshake_data for final transfer completion
	bl	commsSendI
	
@ multiboot!
	
	mov	r0, r5
	mov	r1, #1
	swi	0x25	@ swi - multiboot
	
	add	sp, #0x4C

@ now we gotta upload the image to the bootloader :)
#ifndef nocash
	bl	transferGame
#endif
	
	pop	{r4,r5,r6,pc}	@ return THUMB!

.thumb_func
@--------------------------------------------------------------
commsSendI:
@--------------------------------------------------------------

@ write data
	strh	r0, [r2]

@ read siocnt

	@ldrh	r0, [r3]
	@mov	r1, #SIO_START
	@orr	r0, r1
	ldr	r0,=SIOCNT_CTRL|SIO_START

@ start transfer

	strh	r0, [r3]

@ wait until transfer is complete

1:	ldrh	r0, [r3]
	lsr	r0, #8
	bcs	1b
	
@ delay (36us) (604 cycles)
	
	mov	r1, #52		@@@@@@@@@
1:	sub	r1, #1		@@@@@@@@@
	bne	1b		@@@@@@@@@

	ldr	r0,=SIOMULTI1
	ldrh	r0, [r0]
	bx	lr

.thumb_func
@---------------------------------------------------------------
commsDelay:
@---------------------------------------------------------------
	@ delay 16ms
	ldr	r0,=70000
1:	nop
	sub	r0, #1
	bne	1b
	bx	lr

.thumb_func
@-----------------------------------
transferGame:
@-----------------------------------
	
@ delay a few cycles

	push	{lr}

@ ok, *dont* switch modes while the other gba is 'booting'
@ wait until multiplayer mode is disabled

@	mov	r1, #16		@@@@@@@@@
@1:	sub	r1, #1		@@@@@@@@@@
@	bne	1b		@@@@@@@@@

	@ switch to normal mode
	ldr	r4,=0x4000120

@ wait for SD to become low

1:	ldr	r1, [r4, #0x08]
	lsr	r1, #4		@ shift out SD terminal
	bcs	1b

	
	ldr	r1,=(1<<12)+(1<<0)
	strh	r1, [r4, #0x08]	@ siocnt
	
@ wait for SD to become low
@1:	ldrh	r1, [r4, #0x14]	@ RCNT
@	lsr	r1, #2	@ bit1 == SD
@	bcs	1b
	
	mov	r3, #1		@ transfer 256kb (65536 words)
	lsl	r3, #16
	sub	r3, #48	@ minus 0xc0 bytes
@	str	r3, [r4, #0x00] @ siodata32
	
	ldr	r5,=(1<<12)+(1<<0)+SIO_START 
@	strh	r5, [r4, #0x08]
	
@1:	ldrh	r1, [r4, #0x08]
@	lsr	r1, #8
@	bcs	1b

@	mov	r0, #60		@@@@@@@@@
@	lsl	r0, #1
@1:	sub	r0, #1		@@@@@@@@@@
@	bne	1b		@@@@@@@@@

	ldr	r2,=0x20000C0
	
.transfer_words:

	ldmia	r2!, {r0}
	str	r0, [r4, #0x00]	@ siodata32

	str	r5, [r4, #0x08]	@ start siocnt
	
1:	ldrh	r0, [r4, #0x08]
	lsr	r0, #8
	bcs	1b

	sub	r3, #1
	bne	.transfer_words
	
	pop	{pc}

colorcrash:
	ldr	r0,=0x5000000
	ldr	r1,=0x1f001f
	str	r1, [r0]
	mov	r0, #1
	mov	r1, #0
	swi	0x04
