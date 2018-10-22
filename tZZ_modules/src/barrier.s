.section .text
.global memory_barrier
memory_barrier:
	mov	r0, #0 @ <Rd> should be zero (SBZ) for this
	mcr	p15, 0, r0, c7, c10, 5 @ data memory barrier
	mov	pc, lr

@note: c7 is cache operation register
@note: data-mem-barrier ensures apparent order, NOT completion
@note: data-sync-barrier ensures ALL instructions are completed

.global mmsync_barrier
mmsync_barrier:
	mov	r0, #0 @ <Rd> should be zero (SBZ) for this
	mcr	p15, 0, r0, c7, c10, 4 @ data synchronization barrier
	mov	pc, lr
