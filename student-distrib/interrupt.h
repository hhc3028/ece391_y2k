#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "types.h"

/* The actual handler for the keyboard is defined externally */
extern void keyboard_int_handler();

/* The actual handler for the rtc is defined externally */
extern void rtc_int_handler();

/* The actual handler for the rtc is defined externally */
extern void isr_handler();

/* The handlers for all of the exceptions */
extern void exception_divide();
extern void exception_step();
extern void exception_nmi();
extern void exception_breakpoint();
extern void exception_overflow();
extern void exception_bounds_check();
extern void exception_undefined_op();
extern void exception_missing_copro();
extern void exception_doublefault();
extern void exception_overrun_copro();
extern void exception_TSS();
extern void exception_segment();
extern void exception_stack();
extern void exception_GPF();
extern void exception_page();
extern void exception_interrupt();
extern void exception_copro_err();
extern void exception_aligment();
extern void exception_machine();
extern void exception_reserved();

/* The handlers for all of the system calls */
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

/* Function to return the stack for User */
extern void return_to_user(int32_t new_eip);

#endif /* _INTERRUPT_H*/
