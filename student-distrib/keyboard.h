#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "lib.h"

#define ACK 0xFA
#define RESEND 0xFE
#define RELEASE 0xF0

#define BUF_MAX 128
#define RIGHT_SHFT 0x36
#define LEFT_SHFT 0x2A
#define CAPS 0x3A
#define CTRL 0x1D
#define CTRL_REL 0x9D
#define R_SHFT_REL 0xB6
#define L_SHFT_REL 0xAA
#define BACKSPACE 0x0E
#define ENTER 0x1C
#define L_KEY 0x26
#define MAX_CHAR 90
#define MAX_SCAN 0x58

#define KEY_PORT 0x60
#define STATUS_PORT 0x64
#define KEYBOARD_IRQ 0x01

unsigned char buf[128];
int i;

void initialize_keyboard(void);
void keyboard_getchar(void);
unsigned char getchar(void);
unsigned char getScancode(void);
void keyboard_int_handler(void);
int32_t terminal_read(unsigned char * buffer, int32_t nbytes);
int32_t terminal_write(unsigned char *buffer, int32_t nbytes);
void handle_max_buffer();
void update_cursor(int row, int col);

#endif /* _KEYBOARD_H*/
