.section .boot
boot:
	ldr r0,=0x20200000
@set gpio as output
	mov r1,#1
	lsl r1,#21
	str r1,[r0,#16]
loop:
@clr gpio (on led!)
	mov r1,#1
	lsl r1,#15
	str r1,[r0,#44]
@loop delay
	mov r2,#0x3F0000
wait1:
	sub r2,#1
	cmp r2,#0
	bne wait1
@set gpio (off led!)
	mov r1,#1
	lsl r1,#15
	str r1,[r0,#32]
@loop delay
	mov r2,#0x3F0000
wait2:
	sub r2,#1
	cmp r2,#0
	bne wait2
@infinite loop
	b loop
