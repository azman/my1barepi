.section .text
.global enable_fpu
enable_fpu:
@ from http://wiki.osdev.org/ARM_RaspberryPi#Floating_point_support_-_VFP
	ldr r0, =(0xF << 20)
	mcr p15, 0, r0, c1, c0, 2
@ enable vfp
	mov r3, #0x40000000
@	vmsr fpexc, r3    @ assembler bug? says not supported :(
	.long 0xeee83a10
@	fmxr fpexc, r3    @ or, use this? need to try this!
	mov	pc, lr
