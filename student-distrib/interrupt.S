# interrupt.S - Set up interrupt wrappers

# Consulted http://wiki.osdev.org/Interrupt_Service_Routines on how to write
.globl	keyboard_wrapper
.globl	rtc_wrapper
.globl	isr_wrapper
.globl	exception_divide_wrapper
.globl	exception_step_wrapper
.globl	exception_nmi_wrapper
.globl	exception_breakpoint_wrapper
.globl	exception_overflow_wrapper
.globl	exception_bounds_check_wrapper
.globl	exception_undefined_op_wrapper
.globl	exception_missing_copro_wrapper
.globl	exception_doublefault_wrapper
.globl	exception_overrun_copro_wrapper
.globl	exception_TSS_wrapper
.globl	exception_segment_wrapper
.globl	exception_stack_wrapper
.globl	exception_GPF_wrapper
.globl	exception_page_wrapper
.globl	exception_interrupt_wrapper
.globl	exception_copro_err_wrapper
.globl	exception_alignment_wrapper
.globl	exception_machine_wrapper
.globl	exception_reserved_wrapper
.globl	sys_call_handler
.align	4

# Assembly linkage to the keyboard interrupt handler
keyboard_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call keyboard_int_handler

	popal
	sti
	leave
	iret
	
# Assembly linkage to the rtc interrupt handler
rtc_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call rtc_int_handler

	popal
	sti
	leave
	iret

# Assembly linkage to the interrupt_general_handler
isr_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call isr_handler

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_divide_handler
exception_divide_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_divide

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_step_handler
exception_step_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_step

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_nmi_handler
exception_nmi_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_nmi

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_breakpoint_handler
exception_breakpoint_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_breakpoint

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_overflow_handler
exception_overflow_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_overflow

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_bounds_check_handler
exception_bounds_check_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_bounds_check

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_undefined_op_handler
exception_undefined_op_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_undefined_op

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_missing_copro_handler
exception_missing_copro_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_missing_copro

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_doublefault_handler
exception_doublefault_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_doublefault

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_overrun_copro_handler
exception_overrun_copro_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_overrun_copro

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_TSS_handler
exception_TSS_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_TSS

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_segment_handler
exception_segment_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_segment

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_stack_handler
exception_stack_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_stack

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_GPF_handler
exception_GPF_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_GPF

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_page_handler
exception_page_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_page

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_interrupt_handler
exception_interrupt_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_interrupt

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_copro_err_handler
exception_copro_err_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_copro_err

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_alignment_handler
exception_alignment_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	movl	(%esp), %eax
	call exception_alignment

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_machine_handler
exception_machine_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_machine

	popal
	sti
	leave
	iret

# Assembly linkage to the exeception_reserved_handler
exception_reserved_wrapper:
	pushl	%ebp
	movl	%esp, %ebp
	cli
	pushal

	call exception_reserved

	popal
	sti
	leave
	iret

# Assembly linkage for the system call handler
sys_call_handler:
	cmpl	$9, %eax
	ja		done
	jmp		*operations(, %eax, 4)
done:
	ret
operations:
#	.long	halt, execute, read, write, open, close, getargs, vidmap, set_hanlder, sigreturn