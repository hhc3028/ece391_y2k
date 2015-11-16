/* syscall.c - Implements all of the system calls
 */

/* Header file for syscall functionality */
#include "syscall.h"
/* Include these headers for interrupts and printing */
#include "lib.h"
#include "rtc.h"
/* We use the SET_ENTRY macro from here and some constants*/
#include "x86_desc.h"
#include "keyboard.h"
#include "interrupt.h"
#include "file_system.h"
#include "paging.h"

/* Artificial IRET */
extern uint32_t to_user_space();

/* Indicator for the current process we are on */
static uint8_t open_process = 0;

/* Flags that indicate the currently opened processes */
static uint8_t processes = 0x80;

/*
 *execute()
 *this system call attempts to load and execute a new program, handing off the processor to the new
 *program until it terminates. The command is a space-separated sequence of words. The first word is
 *the file name of the program to be executed, and the rest should be provided to the new program on
 *request via getargs system call. 
 *
 *input: command string
 *output: 
 *		-1 if command cannot be executed,
 *		0-255 if program executes a system halt (return value given by the call to halt)
 *		256 if program dies by an exception
 *
 *
 */

int32_t execute(const uint8_t * command)
{

	uint8_t fname[32];
	uint8_t buffer[4];
	uint32_t i;
	uint32_t entry_point = 0;
	uint8_t magic_num[4] = {0x7f, 0x45, 0x4c, 0x46};
	uint32_t first_space_reached;
	uint32_t length_of_fname;
	uint8_t arg_buffer[BUF_MAX];
	uint8_t bitmask = 0x80;
	uint8_t next_process = 0;
	
	for(i = 0; i < 32; i++) {
		fname[i] = '\0';
	}

	//check to see if the command is invalid
	if(command == NULL)
	{
		return -1;
	}

	//storing the process's name and check for validity
	for(i = 0; command[i] != '\0'; i++)
	{	
		if(command[i] == ' ' && first_space_reached == 0)
		{	//get length of process name and terminate string
			first_space_reached = 1;
			length_of_fname = i;
			fname[i] = '\0';
		}

		else if (first_space_reached == 1)
		{	//store extra arg 
			arg_buffer[i - length_of_fname - 1] = command[i];
		}

		else
		{	//process's name exceed limit
			if(i >= 32 && first_space_reached == 0)
			{
				return -1;
			}
			//store process name
			fname[i] = command[i];
		}
	}
	//terminate string
	arg_buffer[i - length_of_fname - 1] = '\0';

	if(first_space_reached == 0)
		{
			fname[i] = '\0';
		}

	
	//read the first 4 bytes of the file to check if it's executable or not
	//and store in buffer
	if(filesystem_read(fname, 0, buffer, 4) <= 0)
	{
		return -1;
	}

	//checking the magic numbers
	if(strncmp((const int8_t*)buffer, (const int8_t*)magic_num, 4) != 0)
	{
		return -1;
	}

	/* check for open slot for process?*/
/*	while(processes & bitmask) {
		if(bitmask != 0x00) {
			next_process++;
			bitmask >> 1;
		}
		else {
			return -1;
		}
	}*/

	/*set up page directory? */
	new_page_dirct(open_process);
	flush_tlb();

	//instruction start at byte 24-27
	if(filesystem_read(fname, instruction_offset, buffer, 4) <= 0)
	{
		return -1;
	}

	//get the instruction
	for(i = 0; i < 4; i++)
	{
		entry_point = (entry_point | (buffer[i] << 8*i));
	}

	//load process to the starting address
	filesystem_load(fname, process_ld_addr);

	//get the pcb - 8mb is end of kernel, we subtract the stack size of each open process
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	asm volatile ("				\
		movl %%ebp, %0		   ;\
		movl %%esp, %1		   ;\
			" : "=a" ((process_control_block->parent_kbp)), "=b" ((process_control_block->parent_ksp))	\
			  : /* no inputs */																			\
			  : "ebp", "esp");																			\

	if(open_process == 0)
	{
		process_control_block->parent_process_number = 0;
		process_control_block->process_number = 0;
	}

	else
	{	//set parent process number of the new process to the parent 
		process_control_block->parent_process_number = ((pcb_t *)(process_control_block->parent_ksp & offset))->process_number;
		//mark that the parent process has sub process
		((pcb_t *)(process_control_block->parent_ksp & offset))->has_child = 1;
	}

	//initialize the file descritor in PCB
	for(i = 0; i < 8; i++)
	{
		process_control_block->fd[i].inode_ptr = NULL;
		process_control_block->fd[i].flags = NOT_IN_USE;
		process_control_block->fd[i].file_position = 0;
		process_control_block->fd[i].fop_ptr.read = NULL;
		process_control_block->fd[i].fop_ptr.write = NULL;
		process_control_block->fd[i].fop_ptr.close = NULL;
		process_control_block->fd[i].fop_ptr.open = NULL;
	}

	//store exxtra arg into pcb buffer for arg
	strcpy((int8_t*)process_control_block->arg_buf, (const int8_t*)arg_buffer);

	//set kernel stack bottom and tss.esp0 to bottom of new kernel stack
	uint32_t temp_ebp, temp_esp;
	temp_ebp = tss.ebp;
	temp_esp = tss.esp;
	tss.esp0 = (_8MB - (_8KB)*(open_process) - 4);

	process_control_block->ebp = temp_ebp;
	process_control_block->esp = temp_esp;
	temp_ebp = _8MB + _4MB - 4;
	//initialize stdin and stdout
	open(open_process, (uint8_t *) "stdin", 0);
	open(open_process, (uint8_t *) "stdout", 0);

	asm volatile (
		"movl	%0, %%ebx					;"
		"jmp	return_to_user				"
			:/* no output */
			:"g" ((entry_point))
			: "ebx");

	return 0;
}


/*
 *halt()
 *
 *this system call terminates a process, returning the specified value to its parent process.
 *We do this by switching back to the parent's kernel stack via PCB information 
 *
 *input: status of process we want to terminate
 *output: status of terminated process
 *
 *
 */



int32_t halt(uint8_t status)
{
	//get the PCB
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	//if the user try to close the only shell being operated on then stop them
	//we can either reset the shell (get the entry point and jump to it) or we can ignore it
	//for now we ignore it
	if(process_control_block->parent_process_number == 0)
	{
		return -1;
		//break;
	}

/* restart the process

	uint8_t buffer[4];
	uint32_t entry_point;

	//checking for validity
	if(filesystem_read((const int8_t *)("shell"), instruction_offset, buffer, 4) <= 0)
	{
		return -1;
	}
	
	//get the entry point
	for( i = 0; i < 4; i++ )
	{
		entry_point |= (buffer[i] << 8 * i);
	}
		
	//jump to instruction
	return_to_user(entry_point);

	}
*/


	
	//set the current process to 0 to mark it done so other process can take this slot


	//set parent process to has no child and update ksp and kbp
	pcb_t * parent_pcb = (pcb_t *)( _8MB - (_8KB)*(process_control_block->parent_process_number + 1));
	parent_pcb->has_child = 0;


	//load page directory of parent
	uint32_t page_addr;
	page_addr = (uint32_t)(&pageDirct[process_control_block->parent_process_number]);
	asm volatile (
			"movl page_addr, %%eax        ;"
			"andl $0xFFFFFFE7, %%eax          ;"
			"movl %%eax, %%cr3                ;"
			"movl %%cr4, %%eax                ;"
			"orl $0x00000090, %%eax           ;"
			"movl %%eax, %%cr4                ;"
			"movl %%cr0, %%eax                ;"
			"orl $0x80000000, %%eax 	      ;"
			"movl %%eax, %%cr0                 "
			: : : "eax", "cc" );
	



	//set kernel stack bottom and tss to parent's kernel stack
	uint32_t temp_ebp, temp_esp;
	temp_ebp = tss.ebp;
	temp_esp = tss.esp;
	tss.esp0 = (_8MB - (_8KB)*(process_control_block->parent_process_number) - 4);



	//status will be lost when we switch stack so push it onto parent stack
	//set ebp and esp to parent's stack
	uint32_t temp_status = status;
/*
	asm volatile ("					\
		movl	%%ebx, %%esp		   ;\
		pushl	%0				   ;\
		movl	%%ecx, %%ebp		   ;\
		popl	%%eax			   ;\
		leave					   ;\
		ret							\
			" : /* no outputs */			


/*																								\
			  : "ebx" (process_control_block->parent_kbp), "ecx" (process_control_block->parent_ksp), "e" ((temp_status))		\
			  : "memory", "ebx", , "ecx", "esp", "ebp", "eax");																	\
*/


	asm volatile(
			"movl %0, %%esp					;"
			"pushl %1						;"
			"movl %0, %%ebp 				;"				
			::"g"(process_control_block->parent_ksp),"g"(temp_status), "g"(process_control_block->parent_kbp));

	asm volatile("popl %eax");

	asm volatile("leave");
	asm volatile("ret");
	return 0;
}


		

/**
 *	Description: open: system_call for open
 *	Inputs: filename: name of the file to open
 *	Outputs: None
 *	Return: 0 on success, -1 on failure
 *	Side Effects: Changes an empty entry in the file array
 */
int32_t open(int32_t fd, const uint8_t* filename, int32_t nbytes){
	//get the pcb - 8mb is end of kernel, we subtract the stack size of each open process
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));
	//iterator to check open processes
	int32_t i = 0;

	/* Check if we have any free file descriptors */
	while(process_control_block->fd[i].flags & IN_USE) {
		if(i >= 8) {
			return -1;
		}
		i++;
	}

	if(strncmp((const int8_t*)filename, "stdin", 5) == 0) {
		if(!(process_control_block->fd[0].flags & IN_USE)) {
			strcpy((int8_t*)process_control_block->filenames[0], "stdin");
			process_control_block->fd[0].fop_ptr.read = terminal_read;
			process_control_block->fd[0].fop_ptr.write = NULL;
			process_control_block->fd[0].fop_ptr.close = terminal_close;
			process_control_block->fd[0].fop_ptr.open = terminal_open;
			process_control_block->fd[0].flags = IN_USE;
			process_control_block->fd[0].file_position = 0;
			process_control_block->fd[0].inode_ptr = NULL;
			process_control_block->file_type[0] = 3;
			return 0;
		}
		else {
			return -1;
		}
	}

	if(strncmp((const int8_t*)filename,"stdout", 6) == 0) {
		if(!(process_control_block->fd[1].flags & IN_USE)) {
			strcpy((int8_t*)process_control_block->filenames[1], "stdout");
			process_control_block->fd[1].fop_ptr.read = NULL;
			process_control_block->fd[1].fop_ptr.write = terminal_write;
			process_control_block->fd[1].fop_ptr.close = terminal_close;
			process_control_block->fd[1].fop_ptr.open = terminal_open;
			process_control_block->fd[1].flags = IN_USE;
			process_control_block->fd[1].file_position = 0;
			process_control_block->fd[1].inode_ptr = NULL;
			process_control_block->file_type[1] = 4;
			return 0;
		}
		else {
			return -1;
		}
	}

	/* Get the dentry for the file */
	dentries_t file;
	if(read_dentry_by_name(filename, &file) == -1) {
		return -1;
	}

	/* Check the type of file and open */
	switch(file.file_type) {
		case '0' :
			return rtc_open(process_control_block, i);
		case '1' :
			return open_dir(process_control_block, i, file);
		case '2' :
			return open_file(process_control_block, i, file);
		default :
			return -1;
	}
}

/**
 *	Description: close: system call for close
 *	Inputs: fd: index in the file array to close
 *	Outputs: None
 *	Return: 0 on success, -1 on failure
 *	Side Effects: Changes an entry in the file array to unused
 */
int32_t close(int32_t fd, const uint8_t* filename, int32_t nbytes) {
	/* Check for invalid file descriptor */
	if(fd < 2 || fd > 7) {
		return -1;
	}

	/* Change the flags to free up the file descriptor */
	/* Might have to call close for each file? */
	//get the pcb - 8mb is end of kernel, we subtract the stack size of each open process
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	if(!(process_control_block->fd[fd].flags & IN_USE)) {
		return -1;
	}
	if(process_control_block->fd[fd].fop_ptr.close == NULL) {
		return -1;
	}
	/* Call the close function for the file entry */
	return process_control_block->fd[fd].fop_ptr.close(process_control_block, fd);
}
/**
 *	Description: read: system call for read
 *	Inputs: fd: index in the file array to read
 *	Outputs: buf: fills the buffer with the bytes read
 *	Return: returns the number of bytes read
 *	Side Effects: None
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
	/* Check for invalid file descriptor */
	if(fd < 0 || fd > 7) {
		return -1;
	}
	//get the pcb - 8mb is end of kernel, we subtract the stack size of each open process
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	if(!(process_control_block->fd[fd].flags & IN_USE)) {
		return -1;
	}
	if(process_control_block->fd[fd].fop_ptr.read == NULL) {
		return -1;
	}

	return process_control_block->fd[fd].fop_ptr.read(fd, buf, nbytes, open_process);
}


/**
 *	Description: read: system call for read
 *	Inputs: fd: index in the file array to read
 *	Outputs: None
 *	Return: 0 on success, -1 on failure
 *	Side Effects: Fills the file given by the fd with nbytes from the buffer
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
	/* Check for invalid file descriptor */
	if(fd < 0 || fd > 7) {
		return -1;
	}
	//get the pcb - 8mb is end of kernel, we subtract the stack size of each open process
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	if(!(process_control_block->fd[fd].flags & IN_USE)) {
		return -1;
	}
	if(process_control_block->fd[fd].fop_ptr.write == NULL) {
		return -1;
	}

	return process_control_block->fd[fd].fop_ptr.write(fd, buf, nbytes);
}
