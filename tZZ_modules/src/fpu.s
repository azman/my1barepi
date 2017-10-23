.section .text
.global enable_fpu
enable_fpu:
	ldr r0, =(0xF << 20)
	mcr p15, 0, r0, c1, c0, 2
	mov r3, #0x40000000
@  vmsr fpexc, r3    # assembler bug? says not supported :(
	.long 0xeee83a10
	mov	pc, lr
