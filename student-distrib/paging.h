#ifndef _PAGING_H
#define _PAGING_H


#define page_size		4096
#define kernel_address 	4194304
#define table_size		1024
#define enable_present	0x01
#define enable_write	0x02
#define enable_user		0x04
#define	enable_through	0x08
#define	disable_cache	0x10
#define enable_4MB		0x80
#define enable_global	0x100

//#define getPDE(x)		(x >> 22)
#define getIndex(x) 	(x >> 12)

// function to initalize paging, used only once
void initialize_paging(void);
// function initializes a page directory for a new task
void new_page_dirct(uint8_t process_number);

// assembly function to enable paging
void enable_paging(void);
// given the address, load the page directory
void load_paging_dirct(uint32_t address);

void flush_tlb();

#endif /* _PAGING_H */
