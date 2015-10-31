/* FILE SYSTEM */

#include "file_system.h"
#include "lib.h"

uint32_t boot_block_addr;					// stores the address of the boot block

uint32_t dir_counter;						// stores the # of dirct entries read

static dentries_t* 		dir_entries;		// starting address of the dir. entries
static inodes_t* 		index_node;			// starting address of the index nodes
static data_block_t* 	data_blocks;		// starting address of the data blocks

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
	index_node = (inodes_t*)(boot_block_addr + sizeof(boot_block_t));
	/* Set the starting address for where the dir. entries */
	dir_entries = (dentries_t*)(boot_block_addr + sizeof(boot_block_t) 
				- (sizeof(dentries_t) * 63));
	
	/* Get the total # of dir. entries */
	boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	int total_index_nodes;
	total_index_nodes = bootBlock -> num_inodes;
	/* Set the starting address of the data blocks */
	data_blocks = (data_block_t*)(boot_block_addr + sizeof(boot_block_t)
					+ (total_index_nodes * sizeof(inodes_t)));

	/* Initialize the dirct entry read counter */
	dir_counter = 0;

	return;
}	


/*
*	read_dentry_by_name
*	DESCRIPTION:	takes the dentry block and uses the file name to name that block
*	INPUT:			fname = file name, dentry = dir. entry
*	OUTPUT:			None
*	RETURN VALUE:	-1 on failure and 0 on success
*	SIDE EFFECT:	sets the file name for the given dir. entry 
*					specified by the second parameter
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	uint32_t name_length = strnlen(fname);			// store the length of the file name
	
	/* check is the file name is of valid size */
	if(1 < name_length && name_length < NAME_SIZE)
	{
		/* Get the total # of dir. entries */
		boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
		int total_dir_entries;
		total_dir_entries = bootBlock -> num_dentries;

		dentries_t curr_dentry;						// variable to keep track of current dentry
		
		/* Cycle through all dir. entries and compare names */
		uint32_t i;
		for(i = 0; i < total_dir_entries; i ++)
		{
			/* Copy the dir entry at index to the curr_dentry variable */
			int bullshit = read_dentry_by_index(i, &curr_dentry);

			/* Call the function from Lib.c to compare the names, 0 for match */
			int is_same = strncmp(fname, (int8_t*)curr_dentry.file_name, NAME_SIZE);
			
			if(is_same == 0)						// if the names match
			{
				*dentry = curr_dentry;				// copy the curr_dentry to the dentry passed into function
				return 0;							// return success (0)
			}
		}		
	}


	return -1;										// return on failure (-1)
}

/*
*	read_dentry_by_index
*	DESCRIPTION:	takes the dentry block and fills in with the given index
*	INPUT:			index = the inode #, dentry = dir. entry
*	OUTPUT:			None
*	RETURN VALUE:	-1 on failure and 0 on success
*	SIDE EFFECT:	sets the inode # to the given dentry passes
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	/* Get the total # of dir. entries */
	boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	int total_dir_entries;
	total_dir_entries = bootBlock -> num_dentries;

	/* Check to see if the index is valid */
	if(total_dir_entries <= index)		// is the index bigger than total amount fo dentries
	{
		return -1;						// if yes, then return fail (-1)
	}

	/* Copy the dentry at index to the dentry passed as parameter */
	*dentry = dir_entries[index];

	return 0;							// return 0 on success of copying
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
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	inodes_t* curr_inode = (inodes_t*) inode;		// holds the ptr to inode
	uint32_t file_length = curr_inode -> length_B;	// store the length of file into variable

	/* Check if the file length is valid */
	if(file_length / 4096 > 1023)					// if the file length isn't within range
	{
		return -1;									// return failure (-1)
	}

	/* Check if the starting point is out of range */
	if(file_length <= offset)						// if the offset starts beyond the file length
	{
		return 0;									// return success and dont include anything into the buffer
	}

	/* Fix the length if it goes over the actual File length */
	uint32_t length_to_read = file_length - offset;				// get the amount to read
	if(length_to_read < length)									// if the length is longer than the amount to read
	{
		length = length_to_read;								// length is changed to the amount to actually read
	}

	
	uint32_t end_index = file_length / 4096;					// get where to end in inode dblock
	uint32_t start_index = offset / 4096;						// get where the index to start in inode dblock

	uint32_t start_dblock = offset - (start_index * 4096);		// get the amount of bytes in the starting dblock
	uint32_t end_dblock = file_length - (end_index * 4096);		// get the amount of bytes in the last dblock

	uint32_t buffer_index = 0;

	uint32_t i;													// variable to keep track of which dblock we are on
	for(i = start_index; i <= end_index; i ++)
	{
		uint32_t dblock_num = index_node -> dblock[i];			// get the data block number

		data_block_t* dblock_addr = data_blocks + (sizeof(data_block_t) * dblock_num)	// get to the data block we need

		if(i == start_index)									// if we are at the starting dblock
		{
			uint32_t x;
			for(x = start_dblock; x < 4096; x ++)				// copy over for the first dblock
			{
				buf[buffer_index] = dblock_addr -> data_nodes[x];
				buffer_index += 1;								// incrememnt the buffer index
			}
		}
		else if(i == end_index)									// if we are at the last dblock
		{
			uint32_t x;
			for(x = 0; x < end_dblock; x ++)
			{
				buf[buffer_index] = dblock_addr -> data_nodes[x];
				buffer_index += 1;
			}
		}
		else													// if we are working with a dblock in the middle
		{
			uint32_t x;
			for(x = 0; x < 4096; x ++)							// copy over for dblock in the middle
			{
				buf[buffer_index] = dblock_addr -> data_nodes[x];
				buffer_index += 1;								// increment the buffer index
			}
		}
	}

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
void test_file_systems(uint32_t address)
{
	/* Initialize the file system with the address passed */
	init_file_systems(address);

	uint32_t temp_index;			// a temp variable to increment through the index
	dentries_t test_dentry;			// a dir. entry variable used for testing

	uint32_t test_num_dentry = boot_block_addr -> num_dentries;
	for(temp_index = 0; temp_index < test_num_dentry; temp_index ++)
	{
		/* Copy the dir. entry corresponging to the index to the teest dentry */
		read_dentry_by_index(temp_index, test_dentry);	

		/* Print out the crudentials for the dir. entrie for that index */
		printf("File Name: %s\n", test_dentry -> file_name);
		printf("File Type: %u\n", test_dentry -> file_type);
		printf("Inode Number: %u\n", test_dentry -> inode_num);
		printf("\n");
	}
	/* Finished runing the Read by Index */
	printf("Done with Reading by index\n");
	printf("\n");

	/* Checking the Read by Name */
	if(read_dentry_by_name("frame1.txt", test_dentry) == 0)
	{
		/* If the function returns in a Success */
		printf("Done with Read by Name\n");
		printf("File Name: %s\n", test_dentry -> file_name); 		
	}

	/* Include Check for Read Data Function */
}

/* File Operations */

int32_t open_file()
{
	return 0;
}

int32_t read_file(const uint8_t* fname, uint32_t offset, uint8_t* buf, uint32_t length)
{
	dentries_t test_dentry;
	uint32_t pass_fail = read_dentry_by_name((uint8_t*) fname, &test_dentry);

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
		return read_data(test_dentry -> inode_num, offset, buf, length);
	}

}

int32_t write_file()
{
	return -1;
}

int32_t close_file()
{
	return 0;
}

/* Directory Operations */
int32_t open_dirct(void)
{
	return 0;
}

int32_t read_dirct(uint8_t* buf)
{
	/* Get the total # of dir. entries */
	boot_block_t* bootBlock = (boot_block_t *) boot_block_addr;
	int total_dir_entries;
	total_dir_entries = bootBlock -> num_dentries;

	/* See if we have already read all the dir entries in the system */
	if(total_dir_entries <= dir_counter)
	{
		dir_counter = 0;		// reset the dir_counter back to 0
		return 0;				// return success (0)
	} 

	/* If there are still dir entries left to read */
	int8_t fname = dir_entries[dir_counter] -> file_name;
	strcpy((int8_t*)buf, (const int8_t*) fname)

	/* Increment the num of dir entries read */
	dir_counter = dir_counter + 1;

	uint32_t name_length = strlen((int8_t*)fname);
	return name_length;

}

int32_t write_dirct(void)
{
	return -1;
}

int32_t close_dirct(void)
{
	return 0;
}
