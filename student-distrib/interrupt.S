# interrupt.S - Set up interrupt wrappers

# Consulted http://wiki.osdev.org/Interrupt_Service_Routines on how to write
.globl	keyboard_wrapper
.globl	rtc_wrapper
.globl	sys_call_handler
.align	4

# Assembly linkage to the keyboard interrupt handler
keyboard_wrapper:
	cli
	pushal

	call keyboard_int_handler

	popal
	sti
	iret
	
# Assembly linkage to the rtc interrupt handler
rtc_wrapper:
	cli
	pushal

	call rtc_int_handler

	popal
	sti
	iret

sys_call_handler:
	cmpl	$9, %eax
	ja		done
	jmp		*operations(, %eax, 4)
done:
	ret
operations:
#	.long	halt, execute, read, write, open, close, getargs, vidmap, set_hanlder, sigreturn

