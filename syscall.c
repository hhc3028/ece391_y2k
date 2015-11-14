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
	uint8_t open_process;
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
	if(filesystem_read(*fname, 0, buffer, 4) != 0)
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
	if(filesystem_read(*fname, instruction_offset, buffer, 4) != 0)
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
		process_control_block->fd[i].inode = 0;
		process_control_block->fd[i].flag = NOT_IN_USE;
		process_control_block->fd[i].fileposition = 0;
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


			
