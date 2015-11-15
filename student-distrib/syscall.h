#define instruction_offset 24
#define process_ld_addr 0x08048000
#define _4KB       0x1000
#define _4MB       0x00400000
#define _8MB       0x00800000
#define _8KB       0x2000
#define offset 	   0xFFFFE000

//according to section 7.2 File System Abstraction 
typedef struct file_descriptor_t {
	fop_t fop_ptr;
	inodes_t * inode_ptr;
	int32_t file_position;
	int32_t flags;
} file_descriptor_t;


typedef struct pcb_t {
	file_descriptor_t fd[8];
	uint8_t filenames[8][32];
	int32_t file_type[8];
	uint32_t parent_kbp;
	uint32_t parent_ksp;
	uint8_t process_number;
	uint8_t parent_process_number;
	uint8_t arg_buf[100];
	uint32_t has_child;
	uint32_t kbp_before_change;
	uint32_t ksp_before_change;
} pcb_t;

typedef struct fop_t {
	int32_t * read;
	int32_t * write;
	int32_t * open;
	int32_t * close;
} fop_t;
