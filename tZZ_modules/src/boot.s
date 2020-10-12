.section .boot
boot:
	mov sp,#0x00008000
	bl main
here:
	b here
.include "boot-libfunc.s"
