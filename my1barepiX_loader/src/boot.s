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
