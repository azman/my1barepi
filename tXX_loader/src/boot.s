@initialize stack to the end of 128MB RAM
.ifndef STACK_INIT
.equ STACK_INIT,0x08000000
.endif
.equ BOOTIRQH,1
.include "boot-interrupt.s"
@ global reset
.global exec_boot
exec_boot:
	mov r0,#0
	str r0,user_irqh
	b load
@ execute code at given address
.global exec_this
exec_this:
	bx r0
@ low-level irq handler
irqh:
@ do we need to save lr? in irq mode lr is banked!
	push {r0-r12,lr}
	bl irq_handler
	pop {r0-r12,lr}
	subs pc,lr,#4
