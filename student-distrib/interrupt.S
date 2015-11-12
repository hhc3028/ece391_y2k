# interrupt.S - Set up interrupt wrappers

#define KERNEL_CS 0x0010
#define KERNEL_DS 0x0018
#define USER_CS 0x0023
#define USER_DS 0x002B

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
		
		movw	%ax,		%ds 	# load the DS with the correct DS (from User)
		movw	%ax, 		%es 	
		movw	%ax, 		%fs
		movw	%ax, 		%gs

		pushl 	%eax				# push the User_DS to the top of the stack

		pushl 	%esp				# push the current stack pointer

		pushf						# push the flags back to the stack

		pushl 	$USER_CS			# push the USER_CS to the top of the stack	
		pushl 	%ebx 				# push the EIP onto the stack

		iret						# return from the interrupt
