#ifndef _RTC_H
#define _RTC_H

#include "types.h"


// the two IO ports for the RTC
#define	INDEX_PORT		0x70
#define DATA_PORT		0x71

// the three status registers
#define STATUS_A		0x0A
#define STATUS_B		0x0B
#define STATUS_C		0x0C

// function to intialize RTC and set freq to 2Hz
uint32_t rtc_initialize();

// function to set the freq of the RTC
uint32_t setFreq(int32_t freq);
void rtc_int_handler(void);

#endif	/* _RTC_H */
