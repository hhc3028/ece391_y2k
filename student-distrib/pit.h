#ifndef PIT_H
#define PIT_H


#define pit_0	       0x40		//channel 0 data port
#define pit_set        0x43		//mode/command register (write only)
#define pit_100hz	   11932	//set to frequency 
#define pit_mode3	   0x34		//00110100 (read low 8 bit only)
#define pit_irq		   0




void pit_init(void);
void pit_interrupt(void); 


#endif /* _PIT_H */
