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
		pageTable[i] = enable_write;
		pageTable[i] |= (i << 12);
		pageDirct[i] = enable_write;
	}

	int x = getIndex(VIDEO);				// get the index to page table
	pageTable[x] |= enable_present;	// set the present bit on
	x = getIndex(VIDEO_1);				// get the index to page table
	pageTable[x] |= enable_present;	// set the present bit on
	x = getIndex(VIDEO_2);				// get the index to page table
	pageTable[x] |= enable_present;	// set the present bit on
	x = getIndex(VIDEO_3);				// get the index to page table
	pageTable[x] |= enable_present;	// set the present bit on


	pageDirct[0] |= (((unsigned int) pageTable) & 0xFFFFF000) | enable_present;		// set the present bit for the video
	pageDirct[1] |= (0x400000) | enable_present | enable_global | enable_4MB;	// set the present bit for the kernel as well as the 4MB bit

	load_paging_dirct((uint32_t)pageDirct);
	enable_paging();
}

void change_task(uint8_t process_number) {

	pageDirct[32] = ((process_number + 2) << 22) | enable_present | enable_4MB | enable_user | enable_write;	// set the present bit for the task as well as the 4MB bit and user bit

	return;
}

/* Function tries to map a page directory entry that contains a desired physical qaddress.
 * If the desired physical address is 0, the function will look for the first free
 * large page and map it to the normal default position, right now default is just large page */
int32_t map_page(uint32_t virtual, uint32_t physical) {
	uint32_t PDE = virtual >> 22;
	uint32_t PTE = (virtual & 0x003FF000) >> 12;
	/* Invalid access memory */
	if(virtual < 0x800000) {
		return -1;
	}
	/* If page table already exists */
	if((pageDirct[PDE] & enable_present) && !(pageDirct[PDE] & enable_4MB)) {
		uint32_t * pagetab = (uint32_t *)(pageDirct[PDE] & 0xFFFFF000);
		pagetab[PTE] = (physical & 0xFFFFF000) | enable_present | enable_write | enable_user;
		return 0;
	}
	/* If page table does not already exit */
	else {
		static uint32_t pageTable1[table_size] __attribute__((aligned (page_size)));
		pageDirct[PDE] = (((unsigned int) pageTable1) & 0xFFFFF000) | enable_present | enable_write | enable_user;
		uint32_t i;
		for(i = 0; i < table_size; i ++)
		{
			pageTable1[i] = 0;
		}
		pageTable1[PTE] = (physical & 0xFFFFF000) | enable_present | enable_write | enable_user;
		return 0;
	}
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
