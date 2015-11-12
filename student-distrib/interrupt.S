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

#---------- STACK ----------#
#	TOP:
#	EIP
#	USER_CS
# 	EFLAGS
# 	USER_DS
return_to_user:
		cli							# mask the interrupts
		movl	4(%esp),	%ebx	# store the EIP into EBX, EBX = EIP

		movl 	$USER_DS,	%eax	# load the segment selectors
		movw	%ax,		%gs 
		movw	%ax, 		%fs
		movw	%ax, 		%ds
		movw	%ax, 		%es

		pushl 	%eax				# push the User_DS to the top of the stack

		pushl 	%0x83FFFF0			# push the user space extended stack pointer

		pushf						# push the flags back to the stack
		popl 	%eax 				# pop back out
		orl 	$0x4200,	%eax	# adjust the flags
		pushl 	%eax				# push the updated flags back onto the stack

		pushl 	$USER_CS			# push the USER_CS to the top of the stack	
		pushl 	%ebx 				# push the EIP onto the stack

		iret						# return from the interrupt
