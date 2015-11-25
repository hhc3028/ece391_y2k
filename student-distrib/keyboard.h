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

#endif /* _KEYBOARD_H*/
