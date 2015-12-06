#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "lib.h"

#define ACK 0xFA
#define RESEND 0xFE
#define RELEASE 0xF0

#define KEY_PORT 0x60
#define STATUS_PORT 0x64
#define KEYBOARD_IRQ 0x01
#define BUF_MAX 1024

//unsigned char buf[BUF_MAX];
int i;


// Terminal Structure to store terminal information
typedef struct terminal_struct {

	uint32_t terminal_id;				// the ID number for the terminal
	uint32_t read_flag;					// indicate whether there arer data to read

	uint8_t	 in_use;					// indicate 0 (no in use) or 1 (currently in use)

	int8_t keyboard_buf[BUF_MAX];		// the keyboard buffer

	uint8_t x_coordinate;
	uint8_t y_coordinate;

	uint32_t video_page;				// address to the video map
	uint32_t storage_page;				

} terminal_struct_t;

void initialize_keyboard(void);
void keyboard_getchar(void);
unsigned char getchar(void);
unsigned char getScancode(void);
void keyboard_int_handler(void);
int32_t terminal_read(const int8_t * fname, int32_t * position, uint8_t* buf, int32_t nbytes);
int32_t terminal_write(int8_t * fname, int32_t * position, const uint8_t* buf, int32_t nbytes);
void handle_max_buffer();
void update_cursor(int row, int col);
int32_t terminal_open();
int32_t terminal_close();


// Checkpoint 5 Functions
uint32_t init_terminals(void);					// initialize the terminal struct
terminal_struct_t *new_terminal(void);			// start a new terminal
void switch_terminal(terminal_struct_t* terminal);	// swtich to another terminal

#endif /* _KEYBOARD_H*/
