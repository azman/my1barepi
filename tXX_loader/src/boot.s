@linker script should place this @0x00008000
.section .boot
boot:
@ these are actually jump instructions
	ldr pc,reset_handler
	ldr pc,undef_handler
	ldr pc,swint_handler
	ldr pc,epref_handler
	ldr pc,edata_handler
	ldr pc,resvd_handler
	ldr pc,doirq_handler
	ldr pc,dofiq_handler
@ these are jump targets
reset_handler: .word init
undef_handler: .word here
swint_handler: .word here
epref_handler: .word here
edata_handler: .word here
resvd_handler: .word here
doirq_handler: .word irqh
dofiq_handler: .word here
init:
@ moving (overriding) interrupt vector table
	mov r0,#0x8000
	mov r1,#0x0000
@ copying the jmp instructions
	ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
	stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
@ copying the jmp targets (hint: r0 & r1 incremented!)
	ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
	stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
@ get to business
	b load
@linker script should place this @0x00200000
.section .load
load:
@initialize stack to the end of 128MB RAM
	mov sp,#0x08000000
	bl main
here:
	b here
@ execute code at given address
.global exec_this
exec_this:
	bx r0
@ duh!
.global enable_irq
enable_irq:
@ equivalent to 'cpsie i'? with 'mov pc,lr'?
	mrs r0,cpsr
	bic r0,r0,#0x80
	msr cpsr_c,r0
	bx lr
user_irqh: .word 0
.global set_user_irqh
set_user_irqh:
	str r0,user_irqh
	bx lr
.global run_user_irqh
run_user_irqh:
	ldr r0,user_irqh
	cmp r0,#0
	bxeq lr
	push {r0-r12,lr}
	blx r0
	pop  {r0-r12,lr}
	subs pc,lr,#4
@ low-level irq handler - will call function named irq_handler
irqh:
@ do we need to save lr? in irq mode lr is banked!
	push {r0-r12,lr}
	bl irq_handler
	pop  {r0-r12,lr}
	subs pc,lr,#4
.include "boot-libfunc.s"
@ global reset
.global reset_bootload
reset_bootload:
	b load
