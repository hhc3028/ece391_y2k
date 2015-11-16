#include "x86_desc.h"
#include "paging.h"
#include "lib.h"
#include "types.h"

/* Redefine from lib.c */
#define VIDEO 0xB8000

static uint32_t pageTable[table_size] __attribute__((aligned (page_size)));
static uint32_t pageDirct[table_size] __attribute__((aligned (page_size)));

// specify whether the privilege is user or kernel
/*typedef enum privilege_lvl
{
	kernelLevel = 1,		// 1 for kernel level privillege
	userLevel = 0			// 0 for user level privillege
}	privilege_lvl;*/

void initialize_paging()
{
	int i;								// variable to iterate and initialize
	for(i = 0; i < table_size; i ++)
	{
		pageTable[i] = 0;
		pageTable[i] |= (i << 12);
		pageDirct[i] = enable_write;
	}

	int x = getIndex(VIDEO);				// get the index to page table
	pageTable[x] |= enable_present;	// set the present bit on


	pageDirct[0] |= (((unsigned int) pageTable) & 0xFFFFF000) | enable_present;		// set the present bit for the video
	pageDirct[1] |= (0x400000) | enable_present | enable_global | enable_4MB;	// set the present bit for the kernel as well as the 4MB bit

	load_paging_dirct((uint32_t)pageDirct);
	enable_paging();
}

void new_page_dirct(uint8_t process_number) {

	pageDirct[32] |= ((process_number + 2) << 22) | enable_present | enable_4MB | enable_user;	// set the present bit for the task as well as the 4MB bit and user bit

	return;
}

void enable_paging()
{
	// found this piece of code on OSDev
	asm volatile (
		"movl %%cr4, %%eax   				;"	
        "orl  $0x10, %%eax  				;"
        "movl %%eax, %%cr4   				;"
                                  		
        "movl %%cr0, %%eax   				;"
        "orl  $0x80000001, %%eax 			;"
        "movl %%eax, %%cr0					"
            : /* no outputs */
            : /* no inputs */
            : "eax"
    );
}

void load_paging_dirct(uint32_t address)
{
	// took this code from OSDev
	asm volatile (
		"movl %0, %%eax						;"
		"andl $0xFFFFFFE7, %%eax			;"
		"movl %%eax, %%cr3					"
			:/* no output */
			: "g" (address)
			: "eax"
	);
}

void flush_tlb()
{
	asm volatile (
		"movl %%cr3, %%eax					;"
		"movl %%eax, %%cr3					"
			:/* no output */
			:/* no input */
			: "eax");
}
