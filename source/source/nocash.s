@*****************************
@* NOCASH BURSTBOOT BACKDOOR *
@*****************************

.global	nocashInstallBurstboot

@*****************************

.text
.thumb
.align 2

.thumb_func
@-------------------------------------------
nocashInstallBurstboot:
@-------------------------------------------
	
	push	{lr}
	
	ldr	r0,=0x4000120
	mov	r1, #0
	str	r1, [r0, #0x14]		@ RCNT = 0
	ldr	r1,=0x5080
	str	r1, [r0, #0x8]		@ SIOCNT = 0x5080
	ldr	r1, msg_boot
	str	r1, [r0, #0x0]		@ SIODATA = "BOOT"
	
	mov	r0, #1<<7		@ IRQ_SERIAL
	ldr	r1,=nocashSerialIRQ	@ pointer to handler
	bl	irqSet			@ install handler
	mov	r0, #1<<7		@ IRQ_SERIAL
	bl	irqEnable		@ enable interrupt
	pop	{r0}
	bx	r0

.arm
.align

@-------------------------------------------
nocashSerialIRQ:
@-------------------------------------------
	
	ldr	r0,=0x4000000
	ldr	r2, [r0, #0x120]
	ldr	r3, msg_brst
	cmp	r2,r3
	bxne	lr

@-------------------------------------------
nocashBurstboot:
@-------------------------------------------
	
	@ disable stuff
	mov	r2, #0
	str	r2, [r0, #0x208]@ IME
	str	r2, [r0, #0xB8]	@ dma0
	str	r2, [r0, #0xC4]	@ dma1
	str	r2, [r0, #0xD0]	@ dma2
	str	r2, [r0, #0xDC]	@ dma3
	str	r2, [r0, #0x84]	@ sound
	
	ldr	r4, msg_okay
	bl	nocashTransfer
	mov	r2, r1
	mov	r3, #0x3000000
	mov	r4, #0

1:	bl	nocashTransfer
	str	r1, [r3], #4
	add	r4, r4, r1
	subs	r2, r2, #4
	bhi	1b
	bl	nocashTransfer
	mov	pc, #0x3000000

@-------------------------------------------
nocashTransfer:
@-------------------------------------------

@ r0 = regs
@ r4 = data
@-------
@ returns r1=received

	str	r4, [r0, #0x120]
	ldr	r1, [r0, #0x128]
	orr	r1, r1, #0x80
	str	r1, [r0, #0x128]

@ wait for data
1:	ldr	r1, [r0, #0x128]
	tst	r1, #0x80
	bne	1b

	ldr	r1, [r0, #0x120]
	bx	lr

.align
msg_boot:
	.byte	'B','O','O','T'

msg_okay:
	.byte	'O','K','A','Y'

msg_brst:
	.byte	'B','R','S','T'
.pool

@*******************************************************************
@*******************************************************************

@***********************
@* NOCASH DEBUG OUTPUT *
@***********************
 
.global nocashPrint

@***********************
 
.section ".iwram", "ax", %progbits
.thumb
.align
 
.thumb_func
@--------------------------------------------
nocashPrint:	@ params = { string }
@--------------------------------------------

@ max string length == 120 bytes

@----------------------------
@ copy string into buffer
@----------------------------

	ldr	r1,=nocashMessageSpace	@ get buffer address
	mov	r2, #0			@ max length = 120 characters
3:	ldrb	r3, [r0,r2]		@ load character
	strb	r3, [r1,r2]		@ store character
	cmp	r3, #0			@ character == NULL?
	beq	nocashSendMessage	@ yes: send message
	add	r2, #1			@ no: increment char count
	cmp	r2, #120		@ loop if < 120 characters
	bne	3b			
	
@----------------------------
@ send message to no$
@----------------------------
	
nocashSendMessage:
	mov	r12,r12		@ first ID
	b	2f		@ skip the text section
	.hword	0x6464		@ second ID
	.hword	0		@ flags
nocashMessageSpace:
	.space	120		@ data
2:	bx	lr		@ exit
	
@--------------------------------------------
.pool
