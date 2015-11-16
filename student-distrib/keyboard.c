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
#define ENTER 0x1C
#define BUF_MAX 128
#define L_KEY 0x26
#define MAX_CHAR 90
#define MAX_SCAN 0x58

static int c_flag = 0;
static int flag = 0;
static int ctrl_flag = 0;
static unsigned char buffer[BUF_MAX];
static unsigned char terminal_buf[BUF_MAX];
static int curr_xcoord = 0;
static int curr_ycoord = 0;
static int allow_read = 0;
char * vid_mem = (char*)VIDEO;

/* Array for the characters without shift or CAPS */
unsigned char scancode[4][MAX_CHAR] =
{
	{	//no special key
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']',
	'\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0,
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{	//for shift keys
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', //14
	0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', //14
	'\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', //14
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', //14
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, //34
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	},
	{	//for caplocks
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', //14
	'\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', //14
	0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', //14
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, //34
	0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
	},
	{	//for caplock + shift
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '{', '}',
	'\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~',
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
	flag = 0;
	c_flag = 0;
	ctrl_flag = 0;
	int a;
	for (a = 0; a < BUF_MAX; a++)
		{buffer[a] = '\0';
		 terminal_buf[a] = '\0';
		}
}
/*
	DESCRIPTION: takes in the buffer value and puts it into the terminal buffer
	Inputs: buffer = buffer val to be put into terminal
			nbytes = size of the data to be put in
	returns nbytes successfully read
*/
int32_t terminal_read(int32_t fd, unsigned char * buf, int32_t nbytes)
{
	int j; //counter variable
	while(!allow_read) //lock until ENTER
	{}
	if(screen_y >= (NUM_ROWS - 2)) //check for overflow
	{
		handle_max_buffer();
	}
	else
	{
		screen_y++;
		screen_x = 0;
	}
	j = 0;
	while(j < BUF_MAX)
	{
		terminal_buf[j] = '\0';
		j++;
	}

	
	for(j = 0; j < nbytes; j++) //set terminal buffer from keyboard buffer
	{
		terminal_buf[j] = buf[j];
		buf[j] = '\0';
	}

	i = 0; //reset buffer and cursor values
	allow_read = 0;
	//screen_x = 0;
	return j; //return nbytes success
}

/* 
	DESCRIPTION: takes in buffer read from terminal read and outputs it onto screen
	inputs 	buf = terminal buf char array
			nbytes = size of data to be outputted
	returns nbytes successfully read
	outputs the buf char array onto screen
	
*/
int32_t terminal_write(int32_t fd, unsigned char * terminal_buf, int32_t nbytes)
{
	int count; //count variable
	for(count = 0; count < nbytes; count++)
	{ //check for overflows and fix it
		if ((screen_y < (NUM_ROWS - 2)) && (screen_x >= (NUM_COLS - 1)))
		{
			screen_x = 0;
			screen_y++;
		}
		else if(screen_x >= (NUM_COLS - 1) && (screen_y >= (NUM_ROWS -2)))
		{
			handle_max_buffer();
		}
		putc(terminal_buf[count]); //print buffer
		//update_cursor(screen_y, screen_x);
	}
	if(screen_y >= (NUM_ROWS - 2)) //check for overflow after finished printing
		handle_max_buffer();

	//update_cursor(screen_y, screen_x);
	return count; //return successful nbytes
}

/**
 *	Description: getScancode: retrieves the key that was pressed 
 *	Inputs: None
 *	Outputs: None
 *	Return: Return the key that was pressed
 *	Side Effects: Reads a byte from port 0x60 or the Keyboard port
 */
unsigned char getScancode()
{
	unsigned char c = inb(KEY_PORT); //read scancode
	if(((c > 0) && (c < MAX_SCAN)) || (c == L_SHFT_REL) || (c == CTRL_REL)) //check for valid scancodes
		return c;
	else
		return 0; //return 0 for invalid key
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
	int j;
	if(i == 0)
	{
		curr_ycoord = screen_y; //set initial starting point for buffer
		curr_xcoord = screen_x;
		if(curr_ycoord > (NUM_ROWS - 2)) //handle if its too low
			handle_max_buffer();
	}
	unsigned char s_code = getScancode();
	switch (s_code)
	{
		/*
	case(RIGHT_SHFT):
		if(flag == 2)
			flag = 3;
		else
			flag = 1;
		s_code = 0x01;
		break;
		*/
	case(LEFT_SHFT): //set flag
		if(flag == 2)
			flag = 3;
		else
			flag = 1;
		s_code = 0x01;
		break;	
		/*	
	case(R_SHFT_REL):
		if(flag == 3)
			flag = 2;
		else 
			flag = 0;
		break;
		*/
	case(L_SHFT_REL): //set flag
		if(flag == 3)
			flag = 2;
		else 
			flag = 0;
		break;
	case(CAPS): //set flag
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
		ctrl_flag = 1; //set flag
		break;
	case(CTRL_REL):
		ctrl_flag = 0;
		break;
	case(L_KEY): // 0x26 is scan value of L
		if(ctrl_flag == 1)
		{
			for(j = 0; j < i; j++)
				buffer[j] = NULL;
			s_code = 0x01; //clear the L scan value so it won't print
			clear();
			screen_x = 0;
			screen_y = 0; //reset the cursor and buffer
			i = 0;
		}
		else if(i < BUF_MAX)
		{
			out = scancode[flag][s_code]; //special case for L since it does not register normally
			buffer[i] = out;
			i++;
		}
		update_cursor(screen_y, screen_x);
			//clear screen
		break;
	case(BACKSPACE):
		if(i > 0) 
		{
			if(screen_x > 0) //just do delete
			{
				screen_x--;
				putc(' ');
				screen_x--;
			}
			else if((screen_x == 0) && (i > 0)) //do delete for overflows
			{
				screen_x = NUM_COLS - 2;
				screen_y--;
				putc(' ');
				screen_x = NUM_COLS - 2;
			}
			buffer[i-1] = '\0';
			i--;
		update_cursor(screen_y, screen_x);		
		}
		break;
	case(ENTER):
		//do the enter implementation here
		//it will need to output bufferfer and clear it after
		allow_read = 1;
		terminal_read(0, buffer, i);
		if(screen_y < (NUM_ROWS - 2))
		{
			putc('\n');
		}
		else
			handle_max_buffer();
		break;
	default:
	if(ctrl_flag == 0)
	{
		out = scancode[flag][s_code]; //fetch char from scancode
		if((out != 0) && (i < BUF_MAX))	//if valid char, print it
		{
			buffer[i] = out;
			i++;
		}
		screen_x = curr_xcoord;
		screen_y = curr_ycoord; //keep track of start of buffer
		for(j = 0; j < i; j++)
		{
			if(screen_x >= (NUM_COLS - 1))
			{
				if(screen_y < (NUM_ROWS - 2))	 //handle cases for scrolling and overflow
				{
					screen_y++;
					screen_x = 0;
				}
				else if (screen_y >= (NUM_ROWS - 2))
				{
					handle_max_buffer();
				}
			}
			putc(buffer[j]);
			//update_cursor(screen_y, screen_x);
		}
	}
		break;
	//update_cursor(screen_y, screen_x);
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

int32_t terminal_close()
{
	return 0; //nothing apparently for this checkpoint
}

/*
	DESCRIPTION When the terminal stuff gets too low on screen and almost out of bounds
				this will slide the screen up by one line to keep things in bounds
	inputs 	none
	outputs slides everything on screen by one up vertically
*/
void handle_max_buffer()
{
	for(screen_y = 0; screen_y < NUM_ROWS; screen_y++) //nested loop to get all positions
	{
		for(screen_x = 0; screen_x < NUM_COLS; screen_x++)
		{
			if(screen_y < (NUM_ROWS - 2))
			{
		    	*(uint8_t *)(vid_mem + ((NUM_COLS*screen_y + screen_x) << 1)) = *(uint8_t *)(vid_mem + ((NUM_COLS * (screen_y + 1) + screen_x) << 1));
		    	*(uint8_t *)(vid_mem + ((NUM_COLS*screen_y + screen_x) << 1) + 1) = *(uint8_t *)(vid_mem + ((NUM_COLS * (screen_y + 1) + screen_x) << 1) + 1);
			}
			else if(screen_y >= (NUM_ROWS - 2))
			{
				*(uint8_t *)(vid_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = ' ';
		    	*(uint8_t *)(vid_mem + ((NUM_COLS*screen_y + screen_x) << 1) + 1) = ATTRIB;
		    } // switch everything up by one row
		}
	}
	//reset the cursor so it will be set properly
	screen_y = NUM_ROWS - 2;
	screen_x = 0;
	if(i != 0)
		curr_ycoord--;
	
	update_cursor(screen_y, screen_x);
}
/*
	DESCRIPTION This keeps in track of cursor
	input row & col = position of the current cursor
	output = sets the cursor to where we are currently typing
*/
 void update_cursor(int row, int col) //from OSdev
 {
    unsigned short position=(row * 80) + col;
 
    // cursor LOW port to vga INDEX register
    outb(0x0F, 0x3D4);
    outb((unsigned char)(position & 0xFF), 0x3D5);
    // cursor HIGH port to vga INDEX register
    outb(0x0E, 0x3D4);
    outb((unsigned char )((position >> 8) & 0xFF), 0x3D5);
 }

int32_t terminal_open()
{
	i = 0;
	flag = 0;
	c_flag = 0;
	ctrl_flag = 0;
	int a;
	for (a = 0; a < BUF_MAX; a++)
		{buffer[a] = '\0';
		terminal_buf[a] = '\0';
		}
	return 0;
}
