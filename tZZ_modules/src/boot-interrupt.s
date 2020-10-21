.ifndef BOOTSECT
.equ BOOTSECT,1
.section .boot
boot:
.endif
.equ SYS_ID,0x0031594D
that:
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
@ system flag starts with "MY1\0"
sys0flag: .word SYS_ID
sys1flag: .word 0
sys2flag: .word 0
sys3flag: .word 0
sys4flag: .word 0
sys5flag: .word 0
sys6flag: .word 0
sys7flag: .word user_irqh
init:
@ will skip interrupt vector table if one is already there...
	ldr r0,=0x0000
	ldr r0,[r0,#((sys0flag-that)<<2)]
	ldr r1,=SYS_ID
	cmp r0,r1
	beq next
@ moving (overriding) interrupt vector table
	mov r0,#0x8000
	mov r1,#0x0000
@ copying the jmp instructions  (*ia r*! => reg Incremented After use)
	ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
	stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
@ copying the jmp targets
	ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
	stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
@ copying the system flags
	ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
	stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
next:
@ continue with the program
	b load
.section .load
load:
.include "boot.s"
@ get system flag
.global get_sysflag
get_sysflag:
	ldr r1,=0x0000
	cmp r0,#8
	bge get_sysflag_done
	mov r0,r0,lsl #2
	add r0,r0,#(sys0flag-that)
	ldr r1,[r0]
get_sysflag_done:
	mov r0,r1
	bx lr
@ set system flag
.global set_sysflag
set_sysflag:
	cmp r0,#8
	bge set_sysflag_done
	mov r0,r0,lsl #2
	add r0,r0,#(sys0flag-that)
	str r1,[r0]
set_sysflag_done:
	bx lr
@ user-defined irq handler
user_irqh: .word 0
@ call this to assign interrupt hanler
.global handle_irq
handle_irq:
	str r0,user_irqh
	bx lr
@ duh! call this to enable interrupt
.global enable_irq
enable_irq:
	mrs r0,cpsr
	@ BIt-Clear => disable IRQ MASK
	bic r0,r0,#0x80
	@ Move-to-Status-Register from ARM reg (_c control field only?)
	msr cpsr_c,r0
	mov pc,lr
@ duh!
.global disable_irq
disable_irq:
	mrs r0,cpsr
	orr r0,r0,#0x80
	msr cpsr_c,r0
	mov pc,lr
.ifndef BOOTIRQH
@ low-level irq handler
irqh:
	push {r0-r12,lr}
	@stmfd sp!,{r0-r12,lr}
	ldr r0,user_irqh
	cmp r0,#0
	beq irqh_done
	ldr lr,=irqh_done
	mov pc,r0
irqh_done:
	pop {r0-r12,lr}
	@ldmfd sp!,{r0-r12,lr}
	subs pc,lr,#4
.endif
