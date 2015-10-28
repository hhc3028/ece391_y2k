/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


/* 
*	Initialize the 8259 PIC 
*  	DESCRIPTION: 	initialized the MASTER and SLAVE PIC
*  	INPUT:			None
*  	OUTPUT:			None
*  	RETURN VALUE:	None
*/
void i8259_init(void)
{	
	// masks interrupts from happening
	uint32_t flags;
	cli_and_save(flags);

	//mask everything while initalizing the PICs
	master_mask = EVERYTHING_MASK;
	slave_mask = EVERYTHING_MASK;
	
	// send the given bitmask to PIC, to mask
	outb(master_mask, MASTER_8259_PORT + OFFSET);
	outb(slave_mask, SLAVE_8259_PORT + OFFSET); 

	
	// initalize the master PIC
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT + OFFSET);
	outb(ICW3_MASTER, MASTER_8259_PORT + OFFSET);
	outb(ICW4, MASTER_8259_PORT + OFFSET);

	// initalize the slave PIC
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + OFFSET);
	outb(ICW3_SLAVE, SLAVE_8259_PORT + OFFSET);
	outb(ICW4, SLAVE_8259_PORT + OFFSET);

	// enable irq for slave
	enable_irq(2);

	// enable interrupts after we are done intializing
	restore_flags(flags);
	sti();
}

/* 
*	Enable (unmask) the specified IRQ  
*  	DESCRIPTION: 	enable the specified IRQ port
*  	INPUT:			the IRQ port to enable on the PIC
*  	OUTPUT:			None
*  	RETURN VALUE:	None
*
*/
void enable_irq(uint32_t irq_num)
{
	// disable interrupts and save flags
	uint32_t flags;
	cli_and_save(flags);

	// if the inputed IRQ is out of range
	if(irq_num > MAX_IRQ)
	{
		// restore flags and reable interrupts
		restore_flags(flags);
		sti();
	}

	// enable the specific IRQ
	// got this code from OSDev
	uint8_t	value;
	uint16_t port;

	if(irq_num < 8)
	{
		port = MASTER_8259_PORT + OFFSET;
		value = master_mask;
		value = value & ~(1 << irq_num);
		master_mask = value;
	}
	else
	{
		port = SLAVE_8259_PORT + OFFSET;
		irq_num = irq_num - MASTER_IRQ;
		value = slave_mask;
		value = value & ~(1 << irq_num);
		slave_mask = value;
	}

	outb(value, port);

	// restore flags and reable interrupts
	restore_flags(flags);
	sti();
}

/* 
*	Disable (mask) the specified IRQ   
*  	DESCRIPTION: 	disables the specified IRQ port
*  	INPUT:			the IRQ port to be disabled
*  	OUTPUT:			None
*  	RETURN VALUE:	None
*
*/
void disable_irq(uint32_t irq_num)
{
	// disable interrupts and save flags
	uint32_t flags;
	cli_and_save(flags);

	// if the inputed IRQ is out of range
	if(irq_num > MAX_IRQ)
	{
		// restore flags and reable interrupts
		restore_flags(flags);
		sti();
	}

	// disables the specific IRQ
	// got this code from OSDev	
	uint8_t value;
	uint16_t port;

	if(irq_num < 8)
	{
		port = MASTER_8259_PORT + OFFSET;
		value = master_mask;
		value = value | (1 << irq_num);
		master_mask = value;
	}
	else
	{
		port = SLAVE_8259_PORT + OFFSET;
		irq_num = irq_num - MASTER_IRQ;
		value = slave_mask;
		value = value | (1 << irq_num);
		slave_mask = value;
	}

	outb(value, port);

	// restore flags and enable interrupts
	restore_flags(flags);
	sti();
}

/* 
*	Send end-of-interrupt signal for the specified IRQ 
*  	DESCRIPTION: 	sends the EOI for the specified IRQ port
*  	INPUT:			the IRQ port that needs a EOI
*  	OUTPUT:			None
*  	RETURN VALUE:	None
*
*/
void send_eoi(uint32_t irq_num)
{
	cli();

	// if the inputed IRQ is out of range
	if(irq_num > MAX_IRQ)
	{
		sti();
	}

	if(irq_num <= 7 && irq_num >= 0) {
		outb(EOI | irq_num, MASTER_8259_PORT);
	}
	// send the EOI for the specific IRQ inputted
	// got this code from OSDev
	else if(irq_num <= MAX_IRQ && irq_num >= 8)
	{
		outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
		outb(EOI + 2, MASTER_8259_PORT);
	}

	sti();
}

