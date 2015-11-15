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

/* File System Utilies */
int32_t read_dentry_by_name(const uint8_t* fname, dentries_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentries_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* File Operations */
int32_t open_file(pcb_t * process_control_block, int32_t file_num, dentries_t file);
int32_t read_file(const uint8_t* fname, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t write_file(void);
int32_t close_file(pcb_t * process_control_block, int32_t file_num);

/*Directory Operations */
int32_t open_dir(pcb_t * process_control_block, int32_t file_num, dentries_t file);
int32_t read_dir(uint8_t* buf);
int32_t write_dir(void);
int32_t close_dir(pcb_t * process_control_block, int32_t file_num);

#endif 	/* _FILESYSTEM_H */

