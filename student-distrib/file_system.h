#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"

/* Magic Number Handler */
#define NAME_SIZE 		32		// the total size of a name
#define BB_RESERVE		52		// the 52B reserved space in the boto block
#define DENTRY_RESERVE	24		// the 24B reserved space in the dir. entry
#define FOUR_KB_SIZE	4096	// 4kB
#define NUM_OF_DENTRY	63		// there are 63 dir. entries stored in boot block
#define PAGE_SIZE		1023
#define BIG_BUF_SIZE	6000
#define INCR_ONE		1
#define SUCCESS			0
#define FAIL			-1

/* FILE TYPES */
#define TYPE_RTC		0		// ignore index node
#define TYPE_DIR		1		// ignore index node
#define TYPE_REGULAR	2		// index node is only meaningful for regular files

/* The structs for the File System types */
typedef struct dentries 							// struct for rest of dir. entries
{
	uint8_t file_name[NAME_SIZE];					// file name (32B)
	uint32_t file_type;								// file type (4B)
	uint32_t inode_num;								// inode #
	uint8_t dentry_reserved[DENTRY_RESERVE];		// 24B reserved
} dentries_t;

typedef struct inodes 					// struct for index nodes
{
	uint32_t length_B;					// length in B
	uint32_t dblock[PAGE_SIZE];				// the total size of datablock for inode
} inodes_t;

typedef struct data_block 				// struct for the data block
{
	uint8_t data_nodes[FOUR_KB_SIZE];	// total size of the data blocks
} data_block_t;

typedef struct boot_block				// struct for the first block in memory
{
	uint32_t num_dentries;				// # of dir. entries
	uint32_t num_inodes;				// # of inodes(N)
	uint32_t num_dataBlocks;			// # of data blocks (D)
	uint8_t bb_reserved[BB_RESERVE];	// the 52B reserved
	dentries_t dentries[NUM_OF_DENTRY];	// 64B dir. entries
} boot_block_t;

/* Initializing the File System in memory */
void init_file_systems(uint32_t address);

/* File System Utilies */
int32_t read_dentry_by_name(const uint8_t* fname, dentries_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentries_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* File Operations */
int32_t open_file(void);
int32_t read_file(const uint8_t* fname, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t write_file(void);
int32_t close_file(void);

/*Directory Operations */
int32_t open_dir(void);
int32_t read_dir(uint8_t* buf);
int32_t write_dir(void);
int32_t close_dir(void);

/* Function to test File Systems */
<<<<<<< HEAD
//void test_file_system(uint32_t address);

#endif 	/* _FILESYSTEM_H */
=======
extern void test_file_systems(const uint8_t* fname);

#endif 	/* _FILESYSTEM_H */

>>>>>>> origin/master
