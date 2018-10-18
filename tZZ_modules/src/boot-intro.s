.section .boot
boot:
	mov sp,#0x00008000
	bl main
here:
	b here

.global get32
get32:
	ldr r0,[r0]
	bx lr

.global put32
put32:
	str r1,[r0]
	bx lr

@loop delay
.global loopd
loopd:
	sub r0,#1
	cmp r0,#0
	bne loopd
	bx lr
