.section .text
.global enable_fpu
enable_fpu:
@ get co-processor cp15 : c1-c0-2 : cpXX access control register
	mrc p15, 0, r0, c1, c0, 2
@ set vfp (cp10/cp11) for privilege AND user mode access
	orr r0, r0, #0x300000 @ cp10
	orr r0, r0, #0xC00000 @ cp11
@ set new config
	mcr p15, 0, r0, c1, c0, 2
@ enable vfp
	mov r0, #0x40000000
@	vmsr fpexc, r0    @ assembler bug? says not supported :(
	.long 0xeee80a10
@	fmxr fpexc,r0 @ also causes compiler error
	mov	pc, lr

@ vmsr - transfer content of arm register to adv. simd and fp system register
@ fmxr - transfer content of arm register to vfp system register
@ > fpexc - floating-point exception register?
@ >       - bit 30: enable bit for vfp

@ arm instruction opcode assembly
@ 0xe
@ bit:31-28 => conditions for exec => 1110 : AL (always execute)
@ 0xe
@ bit:27-24 => 1110 => co-processor reg transfer (or data op)
@ 0xe
@ bit:23-21 => 111 => opcode for?
@ bit:20--- => 0 => to co-processor
@ 0x8
@ bit:19-16 => 1000 => co-processor register (fpexc?)
@ 0x0
@ bit:15-12 => 0000 => ARM register (r0!)
@ 0xa
@ bit:11-08 => 1010 => co-processor number (10?)
@ 0x1
@ bit:07-05 => 000 => opcode_2
@ bit:04--- => 1 => always 1
@ 0x0
@ bit:03-00 => 0000 => co-processor register
