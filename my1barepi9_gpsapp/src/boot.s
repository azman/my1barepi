.section .boot
init:
	mov sp,#0x00008000
	bl main
here:
	b here
