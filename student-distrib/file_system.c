//* FILE SYSTEM */

#include "file_system.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscall.h"

uint32_t boot_block_addr;					// stores the address of the boot block

uint32_t dir_counter;						// stores the # of dirct entries read

static dentries_t* 		dir_entries;		// starting address of the dir. entries
static inodes_t* 		index_node;			// starting address of the index nodes
static data_block_t* 	data_blocks;		// starting address of the data blocks
static uint32_t			is_open = 0;

/*
*	init_file_systems
*	DESCRIPTION:	initialize the address absolute block numbers
*	INPUT:			address - the start of the absolute block number
*	OUTPUT:			None
*	RETURN VALUE:	Nonde
*	SIDE EFFECT:	initialize the boot block, inode, data blocks
*/
void init_file_systems(uint32_t address)
{
	/* Set the address for the start of the absolute block */
	boot_block_addr = address;
	
	/* Set the starting address for where the index nodes start */	
	index_node = (inodes_t*)(boot_block_addr + FOUR_KB_SIZE);
	
	/* Set the starting address for where the dir. entries */
	dir_entries = (dentries_t*)(boot_block_addr + NUM_OF_DENTRY + INCR_ONE);
	
	/* Get the total # of dir. entries */
	boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	int total_index_nodes;
	total_index_nodes = bootBlock->num_inodes;
	
	/* Set the starting address of the data blocks */
	data_blocks = (data_block_t*)(boot_block_addr) + (total_index_nodes + INCR_ONE);

	/* Initialize the dirct entry read counter */
	dir_counter = 0;

	return;
}	

/********** CHECKPOINT 3 FUNCTIONS **********/

/*
*	filesystem_load
*	DESCRIPTION:	Loads the corresponding file into the address passed 
*	INPUT:			fname - the name of the file, address - the starting address of our buffer
*	OUTPUT:			NONE
*	RETURN VALUE:	Return 0 (Success), Return -1 (Fail)
*	SIDE EFFECT:	Loads the file from the corresponding fname into the buffer at the
*					corresponding address
*/
int32_t filesystem_load(const uint8_t* fname, uint32_t address)
{
	if(fname == NULL) 		// check if the file name is valid
	{
		return -1;			// if not, return -1 (Fail)
	}

	if(address == 0)		// if there isnt a valid address
	{
		return -1;			// return -1 (Fail)
	}

	dentries_t fs_dentry;	// local dentry variable used in this function

	// copy the file corresponding to fname into the dentry variable
	int32_t is_valid = read_dentry_by_name((uint8_t*) fname, &fs_dentry);
	if(is_valid == 0)		// if the copy was a success
	{
		// read the data into the address with the coressponding file
		is_valid = read_data(fs_dentry.inode_num, 0, (uint8_t*) address, index_node[fs_dentry.inode_num].length_B);
		if(is_valid >= 0)	// if the copy into the address worked
		{
			return 0;
		}
		return -1;			// if the read data fails, return -1 (Fail)
	}
	return -1;				// if the read by name fails, return -1 (Fail)
}

/*
*	filesystem_open
*	DESCRIPTION:	Sets up the file system with the passed in addess
*	INPUT:			address - the starting address of our file system
*	OUTPUT:			NONE
*	RETURN VALUE:	Return 0 (Success), Return -1 (Fail)
*	SIDE EFFECT:	sets the starting address of the file system with the 
*					parameter address. is there is currently a file open
*					return -1 (Fail)
*/
int32_t filesystem_open(uint32_t address)
{
	// Check if the file system is already open
	if(is_open == 0)					// if the file system is not currently open
	{
		init_file_systems(address);		// initialize the start of the file system
		is_open = 1;					// set is_open to 1, file system is open
		return 0;						// return success
	}
	else								// if the file system is currently open
		return -1;						// return -1 (Failure)
}

/*
*	filesystem_close
*	DESCRIPTION:	Check whether the file system needs closing
*	INPUT:			NONE
*	OUTPUT:			NONE
*	RETURN VALUE:	Return 0 (on Success), Return -1 (on Fail)
*	SIDE EFFECT:	if the file system is currently open, then set is_open to 0
*					closing the file. Otherwise the file system isnt currently open
*/
int32_t filesystem_close(void)
{
	// Check if the file system is currently open
	if(is_open == 1)			// if the current file system is open
	{
		is_open = 0;			// set is_open to 0, file system is closed
		return 0;				// return 0 (Success)
	}
	else						// the file system isnt open yet
		return -1;				// thus close returns -1 (Fail)
}

/*
*	filesystem_read
*	DESCRIPTION:	reads the file given by fname and copies it into the buffer
*	INPUT:			fname - the file name we want to read, offset - where we want 
*					to start reading, buf - the buffer we want to copy to, length - the 
* 					amount we want to read
*	OUTPUT:			NONE
*	RETURN VALUE:	return -1 (Fail), return READ_DATA (the amount of bytes read)
*	SIDE EFFECT:	Copies the file corresponding to fname and copies it into buf
*/
int32_t filesystem_read(const uint8_t* fname, uint32_t position, uint8_t* buf, uint32_t length)
{
	if(buf == NULL)			// check if there is a valid buffer
	{
		return -1;			// if not, return -1 (fail)
	}
	
	if(fname == NULL)		// check if the file name is valid
	{
		return -1;			// if not return -1 (fail)
	}

	dentries_t fs_dentry;	// local variable used in this function only

	// copy the file into the local dentry variable
	int32_t is_valid = read_dentry_by_name((uint8_t*) fname, &fs_dentry);
	// check if the copy into the dentry was successful
	if(is_valid == 0)		// if the copy was a success
	{
		// read the number of bytes read and copy into the buffer
		return read_data(fs_dentry.inode_num, position, buf, length);
	}
	else					// if the copy was unsuccessful
		return -1;			// return -1 (Fail)
}

/*
*	filesystem_write
*	DESCRIPTION:	Does nothing because our file is read only
*	INPUT:			NONE
*	OUTPUT:			NONE
*	RETURN VALUE:	Return 0 (Nothing Happens)
*	SIDE EFFECT:	Our file is read only so write function does nothing
*/
int32_t filesystem_write(void)
{
	return 0;
}


/********** CHECKPOINT 2 FUNCTIONS **********/

/*
*	read_dentry_by_name
*	DESCRIPTION:	takes the dentry block and uses the file name to name that block
*	INPUT:			fname = file name, dentry = dir. entry
*	OUTPUT:			None
*	RETURN VALUE:	-1 on failure and 0 on success
*	SIDE EFFECT:	sets the file name for the given dir. entry 
*					specified by the second parameter
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentries_t* dentry)
{
	uint32_t name_length = strlen((int8_t*)fname);			// store the length of the file name
	
	/* check is the file name is of valid size */
	if(1 < name_length && name_length < NAME_SIZE)
	{
		/* Get the total # of dir. entries */
		boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
		int total_dir_entries;
		total_dir_entries = bootBlock->num_dentries;

		dentries_t curr_dentry;						// variable to keep track of current dentry
		
		/* Cycle through all dir. entries and compare names */
		uint32_t i;
		for(i = 0; i < total_dir_entries; i ++)
		{
			/* Copy the dir entry at index to the curr_dentry variable */
			int bullshit = read_dentry_by_index(i, &curr_dentry);
			if(bullshit == FAIL)						// if bullshit is -1
			{
				return FAIL;							// the copy by index failed
			}

			/* Call the function from Lib.c to compare the names, 0 for match */
			int is_same = strncmp((int8_t*)fname, (int8_t*)curr_dentry.file_name, NAME_SIZE);
			
			if(is_same == SUCCESS)						// if the names match
			{
				*dentry = curr_dentry;				// copy the curr_dentry to the dentry passed into function
				return SUCCESS;							// return success (0)
			}
		}		
	}


	return FAIL;										// return on failure (-1)
}

/*
*	read_dentry_by_index
*	DESCRIPTION:	takes the dentry block and fills in with the given index
*	INPUT:			index = the inode #, dentry = dir. entry
*	OUTPUT:			None
*	RETURN VALUE:	-1 on failure and 0 on success
*	SIDE EFFECT:	sets the inode # to the given dentry passes
*/
int32_t read_dentry_by_index(uint32_t index, dentries_t* dentry)
{
	/* Get the total # of dir. entries */
	boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	int total_dir_entries;
	total_dir_entries = bootBlock->num_dentries;

	/* Check to see if the index is valid */
	if(total_dir_entries <= index)		// is the index bigger than total amount fo dentries
	{
		return FAIL;						// if yes, then return fail (-1)
	}

	/* Copy the dentry at index to the dentry passed as parameter */
	strcpy((int8_t*)dentry->file_name, (const int8_t*)dir_entries[index].file_name);
	dentry->file_type = dir_entries[index].file_type;
	dentry->inode_num = dir_entries[index].inode_num;

	return SUCCESS;							// return 0 on success of copying
}

/*
*	read_data
*	DESCRIPTION:	works like a read system call, starting at offset up to length of 
*					the index node specifies
*	INPUT:			inode = the index node to read from, offset = where to start to read,
*					buf = the buffer to place the # og bytes read, length = where to stop reading
*	OUTPUT:			None
*	RETURN VALUE:	-1 on failure and the # of bytes read if successful
*	SIDE EFFECT:	reads file from index node starting at offset and places the # of bytes read
*					into the buff bfore returning 0 on success
*/
int32_t read_data(uint32_t inode, uint32_t position, uint8_t* buf, uint32_t length)
{
	/*dentries_t curr_dentry;							// holds the ptr to inode
	if(read_dentry_by_index(inode, &curr_dentry) == -1)
	{
		return -1;
	}*/

	inodes_t * curr_inode = &index_node[inode];
	uint32_t file_length = curr_inode->length_B;	// store the length of file into variable

	/* Check if the file length is valid */
	if(file_length > PAGE_SIZE * FOUR_KB_SIZE)					// if the file length isn't within range
	{
		return FAIL;									// return failure (-1)
	}

	/* Check if the starting point is out of range */
	if(file_length <= position)						// if the offset starts beyond the file length
	{
		return SUCCESS;									// return success and dont include anything into the buffer
	}

	/* Fix the length if it goes over the actual File length */
	uint32_t length_to_read = file_length - position;				// get the amount to read
	if(length_to_read < length)									// if the length is longer than the amount to read
	{
		length = length_to_read;								// length is changed to the amount to actually read
	}


	uint32_t start_index = position / FOUR_KB_SIZE;
	uint32_t buffer_index = 0;
	data_block_t * cur_dblock = data_blocks + curr_inode->dblock[start_index];

	uint32_t i;													// variable to keep track of which dblock we are on
	for(i = position; i < length + position; i++) 
	{
		if((i / FOUR_KB_SIZE) > start_index) 
		{
			start_index++;
			cur_dblock = data_blocks + curr_inode->dblock[start_index];
		}
		buf[buffer_index] = cur_dblock->data_nodes[i % FOUR_KB_SIZE];
		//printf("%c", buf[buffer_index]);						// uncomment to print everything in file
		buffer_index++;
	}
	
	printf("\n");
	printf("Length Read: %d\n", length);
	printf("Total Size of File: %u\n", file_length);
	return length;
}

/*
*	test_file_systems
*	DESCRIPTION:	Goes through all the functions in file systems and tests to see
*					if the outputs and return values are correct
*	INPUT:			address - the starting address to set the boot block and 
*					initialize everything else to
*	RETURN VALUE:	Various printf statments depending on the function being tested
*	SIDE EFFECT:	None
*/
void test_file_systems(const uint8_t* fname)
{
	clear();						// clears the screen on the terminal
	uint32_t index;
	dentries_t test_dentry;			// dentry variable to help with testing

	uint8_t testing_buffer[BIG_BUF_SIZE];	// buffer used for testing
	for(index = 0; index < BIG_BUF_SIZE; index ++)
	{
		testing_buffer[index] = NULL;
	}

	/* Testing Init File System Pointers and Struct */
	//boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	/*printf("Number of Dentries: %d\n", bootBlock->num_dentries);
	printf("Number of INodes: %d\n", bootBlock->num_inodes);
	printf("Number of Data Blocks: %d\n", bootBlock->num_dataBlocks);
	printf("Set up: DONE\n");*/

	/* Testing Read by Index */
	/*for(index = 0; index < bootBlock->num_dentries; index ++)
	{
		if(read_dentry_by_index(index, &test_dentry) == SUCCESS)
		{
			printf("File Name: %s ", test_dentry.file_name);
			printf("File Type: %u ", test_dentry.file_type);
			printf("Inode Number: %u", test_dentry.inode_num);
			printf("\n");
		}
		else
		{
			printf("Read by Index Failed :(\n");
		}
	}
	printf("Read by Index: SUCCESS\n");*/

	/* Testing LS Functionality */
	/*for(index = 0; index < bootBlock->num_dentries; index ++)
	{
		read_dir(testing_buffer);
	}*/
	
	
	/* Testing Read by Name */
	if(read_dentry_by_name(fname, &test_dentry) == 0)
	{
		printf("Read by Name: SUCCESS\n");
		printf("File Name: %s ", test_dentry.file_name);
		printf("File Type: %u ", test_dentry.file_type);
		printf("Inode Number: %u\n", test_dentry.inode_num);
		printf("\n");
	}
	else
	{
		printf("Read by Name: FAIL\n");
	}

	/* Testing Read Data */
	if(read_data(test_dentry.inode_num, 0, testing_buffer, 6000) > 0)
	{
		printf("Read Data: SUCCESS\n");
	}
	else
	{
		printf("Read Data: FAIL\n");
	}

}



/* File Operations */

int32_t read_file(int32_t fd, uint8_t* buf, int32_t length, int32_t open_process)
{
	pcb_t * process_control_block = (pcb_t *)(_8MB - (_8KB)*(open_process + 1));
	uint32_t position = process_control_block->fd[fd].file_position;
	int8_t * fname = process_control_block->filenames[fd];
	
	dentries_t test_dentry;
	
	uint32_t pass_fail = read_dentry_by_name((uint8_t *) fname, &test_dentry);

	if(fname == NULL)		// if there is no filename passed
	{
		return -1;			// return failure (-1)
	}
	
	else if(buf == NULL)	// if there is no buffer pasted
	{
		return -1;			// return failure (-1)
	}

	/* If there is no file with the passed in fname */
	else if(pass_fail == -1)
	{
		return -1;
	}

	else
	{
		/* If the File is a regular file, call read data */
		if(test_dentry.file_type == TYPE_REGULAR)
		{
			return read_data(test_dentry.inode_num, position, buf, length);
		}
		/* If the file is a directory */
		else if(test_dentry.file_type == TYPE_DIR)
		{
			return read_dir(fd, buf, length, open_process);
		}
		/* Otherwise just return 0 */
		else
		{
			return 0;
		}
	}
	return 0;
}

int32_t write_file(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

int32_t close_file(pcb_t * process_control_block, int32_t file_num)
{
	strcpy((int8_t*)process_control_block->filenames[file_num], NULL);
	process_control_block->fd[file_num].fop_ptr.read = NULL;
	process_control_block->fd[file_num].fop_ptr.write = NULL;
	process_control_block->fd[file_num].fop_ptr.close = NULL;
	process_control_block->fd[file_num].fop_ptr.open = NULL;
	process_control_block->fd[file_num].flags = NOT_IN_USE;
	process_control_block->fd[file_num].file_position = 0;
	process_control_block->fd[file_num].inode_ptr = NULL;
	process_control_block->file_type[file_num] = -1;
	return 0;
}

int32_t open_file(pcb_t * process_control_block, int32_t file_num, dentries_t file)
{
	strcpy((int8_t*)process_control_block->filenames[file_num], file.file_name);
	process_control_block->fd[file_num].fop_ptr.read = read_file;
	process_control_block->fd[file_num].fop_ptr.write = write_file;
	process_control_block->fd[file_num].fop_ptr.close = close_file;
	process_control_block->fd[file_num].fop_ptr.open = open_file;
	process_control_block->fd[file_num].flags = IN_USE;
	process_control_block->fd[file_num].file_position = 0;
	process_control_block->fd[file_num].inode_ptr = &index_node[file.inode_num];
	process_control_block->file_type[file_num] = 2;
	return 0;
}

/* Directory Operations */

int32_t read_dir(int32_t fd, uint8_t* buf, int32_t nbytes, int32_t open_process)
{
	/* Get the total # of dir. entries */
	boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	int total_dir_entries;
	total_dir_entries = bootBlock->num_dentries;

	/* Check if we have already read all the directories */
	if(total_dir_entries <= dir_counter)		// if we already read all directories
	{
		dir_counter = 0;						// reset the directory read counter back to 0
		return 0;								// return success (0)
	}	

	strcpy((int8_t*) buf, (const int8_t*) dir_entries[dir_counter].file_name);
	printf("File Name: %s\n", dir_entries[dir_counter].file_name);
	dir_counter = dir_counter + 1;				// increment the num of directories read

	uint32_t length_read = strlen((const int8_t*) buf);
	return length_read;
}

int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

int32_t close_dir(pcb_t * process_control_block, int32_t file_num)
{
	strcpy((int8_t*)process_control_block->filenames[file_num], NULL);
	process_control_block->fd[file_num].fop_ptr.read = NULL;
	process_control_block->fd[file_num].fop_ptr.write = NULL;
	process_control_block->fd[file_num].fop_ptr.close = NULL;
	process_control_block->fd[file_num].fop_ptr.open = NULL;
	process_control_block->fd[file_num].flags = NOT_IN_USE;
	process_control_block->fd[file_num].file_position = 0;
	process_control_block->fd[file_num].inode_ptr = NULL;
	process_control_block->file_type[file_num] = -1;
	return 0;
}

/* Directory Operations */
int32_t open_dir(pcb_t * process_control_block, int32_t file_num, dentries_t file)
{
	strcpy((int8_t*)process_control_block->filenames[file_num], file.file_name);
	process_control_block->fd[file_num].fop_ptr.read = read_dir;
	process_control_block->fd[file_num].fop_ptr.write = write_dir;
	process_control_block->fd[file_num].fop_ptr.close = close_dir;
	process_control_block->fd[file_num].fop_ptr.open = open_dir;
	process_control_block->fd[file_num].flags = IN_USE;
	process_control_block->fd[file_num].file_position = 0;
	process_control_block->fd[file_num].inode_ptr = &index_node[file.inode_num];
	process_control_block->file_type[file_num] = 1;
	return 0;
}
