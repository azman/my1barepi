.ifndef STACK_INIT
.equ STACK_INIT,0x00008000
.endif
.ifndef BOOTSECT
.equ BOOTSECT,1
.section .boot
boot:
.endif
	mov sp,#STACK_INIT
@ enable_fpu...
	mrc p15, 0, r0, c1, c0, 2
	orr r0, r0, #0x300000 @ cp10
	orr r0, r0, #0xC00000 @ cp11
	mcr p15, 0, r0, c1, c0, 2
	mov r0, #0x40000000
	fmxr fpexc,r0
@ get on with it...
	bl main
here:
	b here
@ read 32-bit data from the given 32-bit address
.global get32
get32:
	ldr r0,[r0]
	bx lr
@ write 32-bit data to the given 32-bit address
.global put32
put32:
	str r1,[r0]
	bx lr
@ loop delay: count {wait} time(s)
.global loopd
loopd:
	sub r0,#1
	cmp r0,#0
	bne loopd
	bx lr
@ apply enabling-bits to given 32-bit address
.global setbit32
setbit32:
	mov r2,r0
	ldr r0,[r2]
	orr r0,r0,r1
	str r0,[r2]
	mov pc,lr
@ apply disabling-bits to given 32-bit address
.global clrbit32
clrbit32:
	mov r2,r0
	ldr r0,[r2]
	bic r0,r0,r1
	str r0,[r2]
	mov pc,lr
@ get masked bits for a given 32-bit address
.global getbit32
getbit32:
	ldr r0,[r0]
	and r0,r0,r1
	mov pc,lr
