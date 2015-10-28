#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

unsigned char scancode[128] =
{
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']',
	0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0,
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char getScancode() //from OSDev
{
	unsigned char c=0;
	do {
		if(inb(KEY_PORT)!=c)
		{
			c=inb(KEY_PORT);
			if((c > 0) && (c < 0x58))	
				return c;
			else if(c >= 0x58)
				return 0;
		}
	}while(1);
}

unsigned char getchar() //from OSDev
{
	return scancode[getScancode()]; // Need to initialize this array with the codes
}

// It will read keys from the keyboard
// Translate it to the mapping that will be put in here
void keyboard_getchar()
{
	//code for the key inputs here.
	//need to map it and interpret it
	//then have it so it can output it
	unsigned char out = 0;
	out = getchar();
	if(out != 0)
	{
		putc(out);
	}

}

void keyboard_int_handler()
{
	asm volatile ("pushal");

	keyboard_getchar();
	
	send_eoi(1);

	asm volatile ("popal");
}

