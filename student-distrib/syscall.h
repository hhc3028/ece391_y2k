#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"

#define instruction_offset 24
#define process_ld_addr 0x08048000
#define _4KB       	0x00001000
#define _4MB       	0x00400000
#define _8MB       	0x00800000
#define _8KB       	0x00002000
#define offset 	   	0xFFFFE000
#define NOT_IN_USE	0x00000000
#define IN_USE		0x00000001
#define _128MB		0x08000000
#define _132MB		0x08400000


int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);
int32_t func_init(void);
int32_t context_switch(uint8_t cur_task, uint32_t * _esp, uint32_t * _ebp);
int32_t save_pcb(void);
#endif 	/* _SYSCALL_H */
