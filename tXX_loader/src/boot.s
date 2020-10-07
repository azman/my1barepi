@linker script should place this @0x00008000
.section .boot
boot:
	b load
@linker script should place this @0x00200000
.section .load
load:
@initialize stack to the end of 128MB RAM
	mov sp,#0x08000000
	bl main
here:
	b here
.globl exec_this
exec_this:
	bx r0
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
