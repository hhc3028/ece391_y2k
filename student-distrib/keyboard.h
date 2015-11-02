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

unsigned char buf[128];
int i;

void initialize_keyboard(void);
void keyboard_getchar(void);
unsigned char getchar(void);
unsigned char getScancode(void);
void keyboard_int_handler(void);
int32_t terminal_read(unsigned char * buffer, int32_t nbytes);
int32_t terminal_write(unsigned char *buffer, int32_t nbytes);
#endif /* _KEYBOARD_H*/
