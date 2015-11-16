/* idt.c - Implements all functionality with the idt
 */

/* Header file for idt functionality */
#include "idt.h"
/* Include these headers for interrupts and printing */
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
/* We use the SET_ENTRY macro from here and some constants*/
#include "x86_desc.h"
#include "keyboard.h"
#include "interrupt.h"
/* Macros. */
/* Retrieve the bit BIT from FLAGS. */
#define CHECK_FLAG(flags,bit)   ((flags >> bit) & 0x01)

/* Video Memory Information */
/* Uncomment this with other video seconds to add pink screen */
//#define VIDEO 0xB8000
//static char* video_mem = (char *)VIDEO;

/* Linkage to the assembler wrapper for the interrupt handlers */
extern uint32_t rtc_wrapper(void);
extern uint32_t keyboard_wrapper(void);
extern uint32_t	isr_wrapper(void);
extern uint32_t	exception_divide_wrapper(void);
extern uint32_t	exception_step_wrapper(void);
extern uint32_t	exception_nmi_wrapper(void);
extern uint32_t	exception_breakpoint_wrapper(void);
extern uint32_t	exception_overflow_wrapper(void);
extern uint32_t exception_bounds_check_wrapper(void);
extern uint32_t	exception_undefined_op_wrapper(void);
extern uint32_t	exception_missing_copro_wrapper(void);
extern uint32_t	exception_doublefault_wrapper(void);
extern uint32_t	exception_overrun_copro_wrapper(void);
extern uint32_t	exception_TSS_wrapper(void);
extern uint32_t	exception_segment_wrapper(void);
extern uint32_t	exception_stack_wrapper(void);
extern uint32_t	exception_GPF_wrapper(void);
extern uint32_t	exception_page_wrapper(void);
extern uint32_t	exception_interrupt_wrapper(void);
extern uint32_t	exception_copro_err_wrapper(void);
extern uint32_t	exception_alignment_wrapper(void);
extern uint32_t	exception_machine_wrapper(void);
extern uint32_t	exception_reserved_wrapper(void);
extern uint32_t sys_call_handler(void);

/* Constants */
/* Constant for a present exception */
#define PRES_EXCEPT 0x97
/* Constant for a not present exception(reserved) */
#define RESERVED 0x17
/* Constant for a generic unitialized interrupt */
#define GENERIC 0x06
/* Constant for a present interrupt */
#define INTERR 0x96
/* Constant for a system call */
#define SYS_CALL 0xF7

/* Declare local function to help set up interrupt entries in table */
static void IDT_ENTRY_SETUP(idt_desc_t * str, uint32_t handler, uint16_t sel, uint8_t flags);
int i;
/* Consulted www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html, section 4.4.4*/
/* This handler if generic interrupts */
void isr_handler() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	printf("received interrut");
	while(1);
}

/* Handler for the divide by zero exception. */
void exception_divide() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Division by zero exception occured.\n");
	while(1);
}

/* Handler for a single step in debugger exception. */
void exception_step() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Single step in debugger exception occured.\n");
	while(1);
}

/* Handler for the non-maskable interrupt exception. */
void exception_nmi() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Non-maskable interrupt exception occured.\n");
	while(1);
}

/* Handler for the breakpoint in debugger exception. */
void exception_breakpoint() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Breakpoint in debugger exception occured.\n");
	while(1);
}

/* Handler for the overflow exception. */
void exception_overflow() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Overflow exception occured.\n");
	while(1);
}

/* Handler for the bounds check exception. */
void exception_bounds_check() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Bounds check exception occured.\n");
	while(1);
}

/* Handler for the undefined opcode exception. */
void exception_undefined_op() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Undefined Operation Code exception occured.\n");
	while(1);
}

/* Handler for the no coprocessor exception. */
void exception_missing_copro() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("No coprocessor exception occured.\n");
	while(1);
}

/* Handler for the double fault exception. */
void exception_doublefault() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Double fault exception occured.\n");
	while(1);
}

/* Handler for the coprocessor segment overrun exception. */
void exception_overrun_copro() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Coprocessor segment overrun exception occured.\n");
	while(1);
}

/* Handler for the invalid task state segment exception. */
void exception_TSS() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Invalid task state segment exception occured.\n");
	while(1);
}

/* Handler for the segment not present exception. */
void exception_segment() {
	/* Clear the screen. */
	//clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Segment not present exception occured.\n");
	while(1);
}

/* Handler for the stack segment overrun exception. */
void exception_stack() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Stack segment overrun exception occured.\n");
	while(1);
}

/* Handler for the general fault protection exception. */
void exception_GPF() {
	int32_t error;
	asm volatile (
		"movl %%eax, %0"
            : "=g" (error)
            : /* no inputs */
            : "eax"
    );
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("General fault protection exception occured.\n");
	printf("%x selector accessed\n", error);
	while(1);
}

/* Handler for the page fault exception. */
void exception_page() {
	int32_t error;
	asm volatile (
		"movl %%eax, %0"
            : "=g" (error)
            : /* no inputs */
            : "eax"
    );
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Page fault exception occured.\n");
	printf("%x exception occurred\n", error);
	while(1);

}

/* Handler for the unknwon interrupt exception. */
void exception_interrupt() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Unknown interrupt exception occured.\n");
	while(1);
}

/* Handler for the coprocessor error exception. */
void exception_copro_err() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Coprocessor error exception occured.\n");
	while(1);
}

/* Handler for the alignment check exception. */
void exception_alignment() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Alignment check exception occured.\n");
	while(1);
}

/* Handler for the machine check exception. */
void exception_machine() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Machine check exception occured.\n");
	while(1);
}

/* Handler for a reserved exception. */
void exception_reserved() {
	/* Clear the screen. */
	clear();	
	//for(i = 0; i < 4000; i++)
	//{*(uint8_t *)(video_mem + (i << 1) + 1) = 0xDF;}
	/* Print out the error message for the exception */
	printf("Reserved exception occured.\n");
	while(1);
}

/**
 *	Description: init_idt: Initializes the idt on startup
 *	Inputs: None
 *	Outputs: None
 *	Return: None
 *	Side Effects: Chains the exception handlers and initializes
 *	entries for all interrupts.
 */
void init_idt() {
	/* Consulted www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html, section 4.4.2*/
	/* We set up the first 32 IDT entries for exceptions sequentially.  We do it in this manner because
	 * we have unique handlers for our exceptions and so we cannot intuitively use a loop except for 
	 * some parts which we sneak in */
	/* Variable for iterating through the idt */
	int i = 0;
	/* Initialize Intel defined exceptions */
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_divide_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_step_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_nmi_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_breakpoint_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_overflow_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_bounds_check_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_undefined_op_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_missing_copro_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_doublefault_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_overrun_copro_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_TSS_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_segment_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_stack_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_GPF_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_page_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_interrupt_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_copro_err_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_alignment_wrapper, KERNEL_CS, PRES_EXCEPT);
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)exception_machine_wrapper, KERNEL_CS, PRES_EXCEPT);
	/* Initialize the next reserved idt descriptors which occupies 0x13 to 0x1F of the IDT */
	for(i = i; i < 0x20; i++) {
		IDT_ENTRY_SETUP(&idt[i], (uint32_t)exception_reserved_wrapper, KERNEL_CS, RESERVED);
	}
	/* Initialize the next idt descriptors to be generic non-present interrupts up to 0x80 */
	for(i = i; i < 0x80; i++) {
		IDT_ENTRY_SETUP(&idt[i], (uint32_t)isr_wrapper, KERNEL_CS, GENERIC);
	}
	/* IDT entry for system calls */
	IDT_ENTRY_SETUP(&idt[i++], (uint32_t)sys_call_handler, KERNEL_CS, SYS_CALL);
	/* Initialize the rest of the idt descriptors to be generic non-present interrupts up to 0xFF */
	for(i = i; i < 0xFF; i++) {
		IDT_ENTRY_SETUP(&idt[i], (uint32_t)isr_handler, KERNEL_CS, GENERIC);
	}

	/* This section may be used to initialize interrupts we know ahead of time */

	IDT_ENTRY_SETUP(&idt[0x28], (uint32_t)rtc_wrapper, KERNEL_CS, INTERR);

	IDT_ENTRY_SETUP(&idt[0x21], (uint32_t)keyboard_wrapper, KERNEL_CS, INTERR);

	/* Set up the idtr to point to the idt */
	lidt(idt_desc_ptr);
}

/**
 *	Description: IDT_ENTRY_SETUP: setups up a descriptor for the idt based on the given inputs
 *	Inputs: str: The given idt descriptor we must set up
 *			handler: The location of the handler for the given interrupt
 *			sel: The segment the interrupt is located in
 *			flags: Given string of 8 bits that determine the attributes of the interrupt
 *	Outputs: None
 *	Return: None
 *	Side Effects: Writes to the interrupt descriptor table the desired interrupt descriptor
 *	entries for all interrupts.
 */
static void IDT_ENTRY_SETUP(idt_desc_t * str, uint32_t handler, uint16_t sel, uint8_t flags) {
	/* Consulted www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html, section 4.4.2*/
	SET_IDT_ENTRY((*str), handler);
	str->seg_selector = sel;
	/* This bit is always 0 */
	str->reserved4 = 0;
	/* Set the elements of the IDT descriptor to be what is described by flags */
	str->reserved3 = CHECK_FLAG(flags, 0);
	str->reserved2 = CHECK_FLAG(flags, 1);
	str->reserved1 = CHECK_FLAG(flags, 2);
	str->reserved0 = CHECK_FLAG(flags, 3);
	str->size = CHECK_FLAG(flags, 4);
	/* This call requires two bits so we do flag checks with one of them shifted */
	str->dpl = CHECK_FLAG(flags, 6) << 1 | CHECK_FLAG(flags, 5);
	str->present = CHECK_FLAG(flags, 7);
}
