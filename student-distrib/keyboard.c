#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

#define RIGHT_SHFT 0x36
#define LEFT_SHFT 0x2A
#define CAPS 0x3A
#define CTRL 0x1D
#define CTRL_REL 0x9D
#define R_SHFT_REL 0xB6
#define L_SHFT_REL 0xAA
#define BACKSPACE 0x0E

static int c_flag = 0;
static int flag = 0;
static int ctrl_flag = 0;
static char buf[128] = '\0';

/* Array for the characters without shift or CAPS */
unsigned char scancode[4][90] =
{
	{	//no special key
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']',
	0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0,
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{	//for shift keys
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', //14
	0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', //14
	0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', //14
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', //14
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, //34
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	},
	{	//for caplocks
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', //14
	0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', //14
	0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', //14
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, //34
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
	},
	{	//for caplock + shift
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '{', '}',
	0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~',
	0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, '*',
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0,
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};

/**
 *	Description: initialize_keyboard: enables the PIC to allow keyboard
 *				 interrupts
 *	Inputs: str: None
 *	Outputs: None
 *	Return: None
 *	Side Effects: Changes the irq mask to include the keyboard
 */
void initialize_keyboard() {
	/* Enable the irq of the PIC for the keyboard */
	enable_irq(KEYBOARD_IRQ);
	i = 0;
}

/**
 *	Description: getScancode: retrieves the key that was pressed 
 *	Inputs: None
 *	Outputs: None
 *	Return: Return the key that was pressed
 *	Side Effects: Reads a byte from port 0x60 or the Keyboard port
 */
unsigned char getScancode() //from OSDev
{
	unsigned char c = inb(KEY_PORT);
	if((c > 0) && (c < 0x58) || (c == R_SHFT_REL) || (c == L_SHFT_REL) || (c == CTRL_REL))
		return c;
	else
		return 0;
}

/**
 *	Description: keyboard_getchar : retrieves a typed character from
 *				 the keyboard and echos it onto the screen 
 *	Inputs: None
 *	Outputs: None
 *	Return: None
 *	Side Effects: Echos a typed character onto the screen
 */
void keyboard_getchar()
{
	//code for the key inputs here.
	//need to map it and interpret it
	//then have it so it can output it
	unsigned char out = 0;
	char s_code = getScancode();
	switch (s_code)
	{
	case(RIGHT_SHFT || LEFT_SHFT):
		if(flag == 2)
			flag = 3;
		else
			flag = 1;
		break;
	case(R_SHFT_REL):
		if(flag == 3)
			flag = 2;
		else 
			flag = 0;
		break;
	case(L_SHFT_REL):
		if(flag == 3)
			flag = 2;
		else 
			flag = 0;
		break;
	case(CAPS):
		if(c_flag == 0)
		{
			if(flag == 1)
				flag = 3;
			else
				flag = 2;
			c_flag = 1;
		}
		else
		{
			if(flag == 3)
				flag = 1;
			else 
				flag = 0;
			c_flag = 0;
		}
		break;
	case(CTRL):
		ctrl_flag = 1;
		break;
	case(CTRL_REL):
		ctrl_flag = 0;
		break;
	case(0x28): // 0x28 is scan value of L
		if(ctrl_flag == 1)
		{
			s_code = 0x01; //clear the L scan value so it won't print
		}
			//clear screen
		break;
	case(BACKSPACE):
		if(i > 0)
		{
			buf[i] = '\0';
			i--;
		}
		break;
	}
	out = scancode[flag][s_code];
	if((out != 0) && (i < 128))
	{
		buf[i] = out;
		putc(buf[i]);
		i++;
	}

}

/* Interrupt handler for the keyboard */
void keyboard_int_handler()
{
	/* Fetch the character */
	keyboard_getchar();
	/* Check the status port */
	inb(STATUS_PORT);
	/* Signal the interrupt is finished */
	send_eoi(KEYBOARD_IRQ);
}
