.section .boot
init:
	mov sp,#0x00008000
	bl main
here:
	b here
.global memory_barrier
memory_barrier:
	mcr	p15, 0, ip, c7, c5, 0  @ invalidate I cache
	mcr	p15, 0, ip, c7, c5, 6  @ invalidate BTB
	mcr	p15, 0, ip, c7, c10, 4 @ drain write buffer
	mcr	p15, 0, ip, c7, c5, 4  @ prefetch flush
	mov	pc, lr
