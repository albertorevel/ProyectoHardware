.global DoUndef
DoUndef:
     .word 0xE6000010
     mov pc, lr

.global DoDabort
DoDabort:
	ldr r0, =0x0a333333
	str r0, [r0]
    mov pc, lr
