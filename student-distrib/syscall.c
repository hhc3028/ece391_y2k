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

/* Artificial IRET */
extern uint32_t to_user_space(uint32_t entry_point);

/* Indicator for the current process we are on */
static uint8_t open_process = 0;

/* System call for halt */
int32_t halt(uint8_t status) {
	return 0;
}

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
	uint8_t arg_buffer[buf_max];
	


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
	if(read_file(*fname, 0, buffer, 4) != 0)
	{
		return -1;
	}

	//checking the magic numbers
	if(strncmp((const int8_t*)buffer, (const int8_t*)magic_num, 4) != 0)
	{
		return -1;
	}


	/* check for open slot for process?*/

	//instruction start at byte 24-27
	if(read_file(*fname, instruction_offset, buffer, 4) != 0)
	{
		return -1;
	}

	//get the instruction
	for(i = 0; i < 4; i++)
	{
		entry_point = (entry_point | (buffer[i] << 8*i));
	}


	/*set up page directory? */

	//load process to the starting address
	filesystem_load(*fname, process_ld_addr);

	//get the pcb - 8mb is end of kernel, we subtract the stack size of each open process
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process +1));

	//store esp and ebp
	uint32_t ebp, esp;

	asm volatile(movl %%ebp, ebp);
	process_control_block->parent_kbp = ebp;
	asm volatile(movl %%esp, esp);
	process_control_block->parent_ksp = esp;

	if(/*first call process*/)
	{
		process_control_block->parent_process_number = 0;
		process_control_block->process_number = 1;
	}

	else
	{	//set parent process number of the new process to the parent 
		process_control_block->parent_process_number = ((pcb_t *)(esp & offset))->process_number;
		//mark that the parent process has sub process
		((pcb_t *)(esp & offset))->has_child = 1;


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



	//initialize stdin and stdout
	open((uint8_t *) "stdin");
	open((uint8_t *) "stdout");

	//jump to entry point to execute
	to_user_space(entry_point);

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
	}

	if(strncmp((const int8_t*)filename, "stdin", 5)) {
		if(process_control_block->filenames[0] == NULL) {
			strcpy((int8_t*)process_control_block->filenames[0], "stdin");
			process_control_block->fd[0].fop_ptr.read = terminal_read;
			process_control_block->fd[0].fop_ptr.write = NULL;
			process_control_block->fd[0].fop_ptr.close = NULL;
			process_control_block->fd[0].fop_ptr.open = NULL;
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

	if(strncmp((const int8_t*)filename,"stdout", 6)) {
		if(process_control_block->filenames[1] == NULL) {
			strcpy((int8_t*)process_control_block->filenames[1], "stdout");
			process_control_block->fd[1].fop_ptr.read = NULL;
			process_control_block->fd[1].fop_ptr.write = terminal_write;
			process_control_block->fd[1].fop_ptr.close = NULL;
			process_control_block->fd[1].fop_ptr.open = NULL;
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

	/* Call the read for the function */
	/* Check the type of file and close */
	switch(process_control_block->file_type[fd]) {
		case '0' :
			return process_control_block->fd[fd].fop_ptr.read();
		case '1' :
			return process_control_block->fd[fd].fop_ptr.read(buf);
		case '2' :
			return process_control_block->fd[fd].fop_ptr.read(process_control_block->filenames[fd], process_control_block->fd[fd].file_position, buf, nbytes);
		case '3' :
			return process_control_block->fd[fd].fop_ptr.read(buf, nbytes);
		default :
			return -1;
	}
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

	/* Call the read for the function */
	/* Check the type of file and close */
	switch(process_control_block->file_type[fd]) {
		case '0' :
			return process_control_block->fd[fd].fop_ptr.write(buf, nbytes);
		case '1' :
			return process_control_block->fd[fd].fop_ptr.write();
		case '2' :
			return process_control_block->fd[fd].fop_ptr.write();
		case '4' :
			return process_control_block->fd[fd].fop_ptr.write(buf, nbytes);
		default :
			return -1;
	}
}
