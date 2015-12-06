
#include "pit.h"
#include "i8259.h"
#include "syscall.h"
#include "lib.h"

static uint8_t flags = 0x00;
static uint8_t cur_task = 0;

/*
 *this function initialize the pit by first masking interupt, then set the mode and frequency for
 *counter and finally enable interupt on irq. The mode we use is mode 3 - square wave generator 
 *because we want the pic to be able to see the interupt and service us until it is masked. 
 *The frequency is set to 100hz - 10ms.
 *input: none
 *output: none (just init the pit)
 */
void pit_init(void){
	cli();
	outb(pit_mode3, pit_set);	
	outb(pit_100hz & 0x0F, pit_0);
	enable_irq(pit_irq);
	sti();
	flags = 0x01;
}

void pit_interrupt(void){
	uint32_t _esp, _ebp;
	cur_task = (cur_task + 1) % 3;
	if((1 << cur_task) & flags) {
		asm volatile ("pushal");
		save_pcb();
		context_switch(cur_task, &_esp, &_ebp);

		asm volatile(
			"movl	%0, %%eax				;"
			"movl	%%eax, %%esp			;"
			"movl	%1, %%eax				;"
			"movl	%%eax, %%ebp 			;"			
				: /* No Outputs */
				: "g"(_esp), "g"(_ebp)
				: "esp", "ebp", "eax");
		asm volatile ("popal");
		send_eoi(0);
		asm volatile ("iret");


	}

	else
	{
		flags |= (1 << cur_task);
		send_eoi(0);
		execute((uint8_t*)"shell");
	}
}

