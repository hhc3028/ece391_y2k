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
extern uint32_t halt_ret_label();

/* Flags that indicate the currently opened processes */
static uint8_t processes = 0x00;

/* Indicator for the current process we are on */
static uint8_t open_process = 0x00;

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
	uint32_t first_space_reached = 0;
	uint32_t length_of_fname = 0;
	uint8_t arg_buffer[BUF_MAX];
	uint8_t bitmask = 0x80;
	uint8_t new_process = 0;
	
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
	if(first_space_reached == 0) {
		length_of_fname = i - 1;
	}
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

	/* check for open slot for process */
	while(processes & bitmask) {
		if(bitmask != 0x00) {
			new_process++;
			bitmask = bitmask >> 1;
		}
		else {
			return -1;
		}
	}
	processes |= bitmask;
	/*set up page directory? */
	new_page_dirct(new_process);
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
	pcb_t * parent_process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	asm volatile ("				\
		movl %%ebp, %0		   ;\
		movl %%esp, %1		   ;\
			" : "=a" ((parent_process_control_block->ebp)), "=b" ((parent_process_control_block->esp))	\
			  : /* no inputs */																			\
			  : "ebp", "esp");																			\

	pcb_t * process_control_block;

	if(new_process == 0)
	{
		process_control_block = parent_process_control_block;
		process_control_block->parent_process_number = -1;
		process_control_block->process_number = new_process;
		process_control_block->parent_ebp = 0;
		process_control_block->parent_esp = 0;
		process_control_block->has_child = 0;
	}

	else
	{	
		process_control_block = (pcb_t *)(_8MB - (_8KB)*(new_process +1));
		//set parent process number of the new process to the parent 
		process_control_block->parent_process_number = open_process;
		//mark that the parent process has sub process
		parent_process_control_block->has_child++;
		process_control_block->process_number = new_process;
		process_control_block->parent_ebp = parent_process_control_block->ebp;
		process_control_block->parent_esp = parent_process_control_block->esp;
		process_control_block->has_child = 0;
		process_control_block->ebp = tss.ebp;
		process_control_block->esp = tss.esp;
		open_process = new_process;
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
	tss.esp0 = (_8MB - (_8KB)*(open_process));
	//initialize stdin and stdout
	open((uint8_t *) "stdin");
	open((uint8_t *) "stdout");

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

	//set the current process to 0 to mark it done so other process can take this slot
	processes &= ~(0x80 >> open_process);

	//if the user try to close the only shell being operated on then stop them
	//we can either reset the shell (get the entry point and jump to it) or we can ignore it
	//for now we ignore it
	if(process_control_block->parent_process_number == -1)
	{
		/*//get the PCB
		open_process = 0;

		asm volatile(
			"popl	%%ebp				;"
			"popl	%%esi				;"
			"popl	%%edi				;"
			"popl	%%edx				;"
			"popl	%%ecx				;"
			"popl	%%ebx				;"
			"addl	$20, %%esp			"
				: // No Outputs
				: // No Inputs
				: "esp", "ebp", "esi", "edi", "edx", "ecx", "ebx");
		execute((uint8_t*)"shell");*/
		return -1;
	}

	//set parent process to has no child and update ksp and kbp
	pcb_t * parent_pcb = (pcb_t *)( _8MB - (_8KB)*(process_control_block->parent_process_number + 1));
	parent_pcb->has_child--;
	open_process = process_control_block->parent_process_number;


	//load page directory of parent
	new_page_dirct(process_control_block->parent_process_number);
	flush_tlb();
	

	//set kernel stack bottom and tss to parent's kernel stack
	tss.esp0 = (_8MB - (_8KB)*(process_control_block->parent_process_number) - 4);

	//status will be lost when we switch stack so push it onto parent stack
	//set ebp and esp to parent's stack
	asm volatile(
		"movl	%0, %%eax				;"
		"movl	%%eax, %%esp			;"
		"pushl	%1						;"	
		"movl	%2, %%eax				;"
		"movl	%%eax, %%ebp 			;"			
		"popl	%%eax					;"
		"jmp	halt_ret_label			"
			: /* No Outputs */
			: "g"(process_control_block->parent_esp),"g"(status), "g"(process_control_block->parent_ebp)
			: "esp", "ebp", "eax");

	return 0;
}


		

/**
 *	Description: open: system_call for open
 *	Inputs: filename: name of the file to open
 *	Outputs: None
 *	Return: 0 on success, -1 on failure
 *	Side Effects: Changes an empty entry in the file array
 */
int32_t open(const uint8_t* filename){
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
			return 1;
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
		case 0 :
			strcpy((int8_t*)process_control_block->filenames[i], "rtc");
			process_control_block->fd[i].fop_ptr.read = rtc_read;
			process_control_block->fd[i].fop_ptr.write = rtc_write;
			process_control_block->fd[i].fop_ptr.close = rtc_close;
			process_control_block->fd[i].fop_ptr.open = rtc_open;
			process_control_block->fd[i].flags = IN_USE;
			process_control_block->fd[i].file_position = 0;
			process_control_block->fd[i].inode_ptr = NULL;
			return i;
		case 1 :
			strcpy((int8_t*)process_control_block->filenames[i], file.file_name);
			process_control_block->fd[i].fop_ptr.read = read_dir;
			process_control_block->fd[i].fop_ptr.write = write_dir;
			process_control_block->fd[i].fop_ptr.close = close_dir;
			process_control_block->fd[i].fop_ptr.open = open_dir;
			process_control_block->fd[i].fop_ptr.open(&process_control_block->fd[i].inode_ptr, file.inode_num);
			process_control_block->fd[i].flags = IN_USE;
			process_control_block->fd[i].file_position = 0;
			return i;
		case 2 :
			strcpy((int8_t*)process_control_block->filenames[i], file.file_name);
			process_control_block->fd[i].fop_ptr.read = read_file;
			process_control_block->fd[i].fop_ptr.write = write_file;
			process_control_block->fd[i].fop_ptr.close = close_file;
			process_control_block->fd[i].fop_ptr.open = open_file;
			process_control_block->fd[i].fop_ptr.open(&process_control_block->fd[i].inode_ptr, file.inode_num);
			process_control_block->fd[i].flags = IN_USE;
			process_control_block->fd[i].file_position = 0;
			return i;
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
int32_t close(int32_t fd) {
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
	strcpy((int8_t*)process_control_block->filenames[fd], NULL);
	process_control_block->fd[fd].fop_ptr.read = NULL;
	process_control_block->fd[fd].fop_ptr.write = NULL;
	process_control_block->fd[fd].fop_ptr.close = NULL;
	process_control_block->fd[fd].fop_ptr.open = NULL;
	process_control_block->fd[fd].flags = NOT_IN_USE;
	process_control_block->fd[fd].file_position = 0;
	process_control_block->fd[fd].inode_ptr = NULL;

	return 0;
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

	return process_control_block->fd[fd].fop_ptr.read(process_control_block->filenames[fd], &process_control_block->fd[fd].file_position, buf, nbytes);
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
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process + 1));

	if(!(process_control_block->fd[fd].flags & IN_USE)) {
		return -1;
	}
	if(process_control_block->fd[fd].fop_ptr.write == NULL) {
		return -1;
	}

	return process_control_block->fd[fd].fop_ptr.write(process_control_block->filenames[fd], &process_control_block->fd[fd].file_position, buf, nbytes);
}

int32_t getargs(uint8_t* buf, int32_t nbytes)
{
	if(nbytes == 0)				// if nbytes is zero, then do nothing
	{
		return -1;				// return -1 (FAIL)
	}

	if(strlen((int8_t*)buf) == 0)		// check if a valid buffer is passed in
	{
		return -1;				// return -1 (FAIL)
	}


	// get the current process we are on
	pcb_t * curr_process = (pcb_t *)(_8MB - (_8KB)*(open_process +1));
	if(strlen((int8_t*)curr_process->arg_buf) == 0)	// if the argument buffer is empty
	{
		return 0;				// return -1 (FAIL)
	}

	if(strlen((int8_t*)curr_process->arg_buf) > strlen((int8_t*)buf)) //the arg buf is larger than buf
	{
		return -1;
	}
	
	// use lib function to copy n bytes into dest from source
	strncpy((int8_t*)buf, (int8_t*)curr_process->arg_buf, nbytes);

	return 0;					// return 0 (SUCCESS)
}




int32_t vidmap(uint8_t** screen_start)
{
	uint32_t lower_bound = 128 << 20;				// the lower bound for the screen
	uint32_t upper_bound = (132 << 20) - 4;			// the upper bound for the screen
	uint32_t terminal;
	// if the start of the screen is within the lower bound
	if((uint32_t) screen_start >= lower_bound)
	{
	// if the start of the screen is within the upper bound, too
		if((uint32_t) screen_start < upper_bound)
		{

			/*
			// Mapping the 4kB page into video memory
			uint32_t page_table_addr;
			uint32_t page_dir_index;
			uint32_t page_table_index;

			page_dir_index = (256 << 20) / (4 << 20);			// get page directory index
			page_table_index = (256 << 20) % (4 << 20);
			page_table_index = page_table_index / (4 << 10);	// get page table index

			page_table_addr = (uint32_t)
			*/

			pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process) & _8KB);
			terminal = process_control_block->terminal_num;

			new_page_dirct(open_process);
			flush_tlb();

			if(terminal == 0)
				*screen_start = (uint8_t *) VIDEO_1;
			else if (terminal == 1)
				*screen_start = (uint8_t *) VIDEO_2;
			else if (terminal == 2)
				*screen_start = (uint8_t *) VIDEO_3;
			else
				return -1;




		}
	}

	// is screen_start is not within range, return fail
	return -1;
}
