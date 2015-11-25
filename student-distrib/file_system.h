#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"
#include "x86_desc.h"

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
#define NOT_IN_USE	0x00000000
#define IN_USE		0x00000001

/* FILE TYPES */
#define TYPE_RTC		0		// ignore index node
#define TYPE_DIR		1		// ignore index node
#define TYPE_REGULAR	2		// index node is only meaningful for regular files

/* Initializing the File System in memory */
void init_file_systems(uint32_t address);

/* Checkpoint 3 Functions */
int32_t filesystem_load(const uint8_t* fname, uint32_t address);
int32_t filesystem_open(uint32_t address);
int32_t filesystem_close(void);
int32_t filesystem_read(const uint8_t* fname, uint32_t position, uint8_t* buf, uint32_t length);
int32_t filesystem_write(void);

/* File System Utilies */
int32_t read_dentry_by_name(const uint8_t* fname, dentries_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentries_t* dentry);
int32_t read_data(uint32_t inode, uint32_t position, uint8_t* buf, uint32_t length);

/* File Operations */
int32_t open_file(inodes_t ** inode_ptr, int32_t inode_num);
int32_t read_file(const int8_t * fname, int32_t * position, uint8_t* buf, int32_t length);
int32_t write_file(int8_t * fname, int32_t * position, const uint8_t* buf, int32_t length);
int32_t close_file();

/*Directory Operations */
int32_t open_dir(inodes_t ** inode_ptr, int32_t inode_num);
int32_t read_dir(const int8_t * fname, int32_t * position, uint8_t* buf, int32_t length);
int32_t write_dir(int8_t * fname, int32_t * position, const uint8_t* buf, int32_t length);
int32_t close_dir();

#endif 	/* _FILESYSTEM_H */

