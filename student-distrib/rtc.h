#ifndef _RTC_H
#define _RTC_H

#include "types.h"


// the two IO ports for the RTC
#define	INDEX_PORT		0x70
#define DATA_PORT		0x71

// the three status registers
#define STATUS_A		0x8A
#define STATUS_B		0x8B
#define STATUS_C		0x8C

// the IRQ port for the RTC
#define RTC_IRQ			0x08

// the frequency to use for the RTC
#define FREQ			1024

// function to intialize RTC and set freq to 2Hz
uint32_t rtc_initialize();

// function to set the freq of the RTC
uint32_t setFreq(int32_t freq);
void rtc_int_handler(void);

#endif	/* _RTC_H */

//system call for rtc
uint32_t rtc_read(void); 
int32_t rtc_write(int32_t * set_freq, int32_t nbytes);
uint32_t rtc_open(void);
uint32_t rtc_close(void);

//helper function to determine if x is power of 2
int isPowerOfTwo (int32_t x);

