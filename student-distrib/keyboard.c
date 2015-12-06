#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"

#define RIGHT_SHFT 0x36
#define LEFT_SHFT 0x2A
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define CAPS 0x3A
#define CTRL 0x1D
#define ALT 0x38
#define ALT_REL 0xB8
#define CTRL_REL 0x9D
#define R_SHFT_REL 0xB6
#define L_SHFT_REL 0xAA
#define BACKSPACE 0x0E
#define ENTER 0x1C
#define L_KEY 0x26
#define MAX_CHAR 90
#define MAX_SCAN 0x58
#define MAX_TERM 8
#define SPECIAL_KEY 0xE0

// Checkpoint 5
#define NUM_TERMINAL	3				// the number of terminals
#define BUF_MAX 		1024			// taken from keyboard.h
#define VIDEO_1 		0x01000
#define VIDEO_2 		0x03000
#define VIDEO_3 		0x05000
#define _4KB       	0x00001000

uint32_t video_memory[3] = {VIDEO_1, VIDEO_2, VIDEO_3};
terminal_struct_t* sys_terminals;		// about the system's terminal
terminal_struct_t* cur_terminal;		// about the current terminal we are on

static int c_flag = 0;
static int flag = 0;
static int ctrl_flag = 0;
static int alt_flag = 0;
static unsigned char buffer[MAX_TERM][BUF_MAX];
static int curr_xcoord = 0;
static int curr_ycoord = 0;
static int allow_read = 0;
static int max_terminal = 0;
static int curr_terminal = 0;
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
	max_terminal = 0;
	curr_terminal = 0;
	int a;
	for (a = 0; a < BUF_MAX; a++)
		{buffer[0][a] = NULL;
		}
}
/*
	DESCRIPTION: takes in the buffer value and puts it into the terminal buffer
	Inputs: buffer = buffer val to be put into terminal
			nbytes = size of the data to be put in
	returns nbytes successfully read
*/
int32_t terminal_read(const int8_t * fname, int32_t * position, uint8_t* buf, int32_t nbytes)
{
	int j; //counter variable
	while(!allow_read) //lock until ENTER
	{}
	putc('\n');
	/*if(screen_y >= (NUM_ROWS - 2)) //check for overflow
	{
		handle_max_buffer();
	}
	else
	{
		screen_y++;
		screen_x = 0;
	}	*/
	for(j = 0; j < nbytes; j++)
		buf[j] = NULL;
	for(j = 0; j < nbytes; j++) //set terminal buffer from keyboard buffer
	{
		buf[j] = buffer[curr_terminal][j];
		buffer[curr_terminal][j] = NULL;
	}
	j = i;
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
int32_t terminal_write(int8_t * fname, int32_t * position, const uint8_t* buf, int32_t nbytes)
{
	int count; //count variable

	for(count = 0; count < nbytes; count++)
	{ //check for overflows and fix it
	/*	if ((screen_y < (NUM_ROWS - 2)) && (screen_x >= (NUM_COLS - 1)))
		{
			screen_x = 0;
			screen_y++;
		}
		else if(screen_x >= (NUM_COLS - 1) && (screen_y >= (NUM_ROWS -2)))
		{
			handle_max_buffer();
		} */

		putc(buf[count]); //print buffer
		//update_cursor(screen_y, screen_x);
	}
	//if(screen_y >= (NUM_ROWS - 2)) //check for overflow after finished printing
//		handle_max_buffer();
	

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
	//	if(curr_ycoord > (NUM_ROWS - 2)) //handle if its too low
	//		handle_max_buffer();
	}
	unsigned char s_code = getScancode();
	switch (s_code)
	{
		
	case(RIGHT_SHFT):
		if(flag == 2)
			flag = 3;
		else
			flag = 1;
		s_code = 0x01;
		break;
		
	case(LEFT_SHFT): //set flag
		if(flag == 2)
			flag = 3;
		else
			flag = 1;
		s_code = 0x01;
		break;	
			
	case(R_SHFT_REL):
		putc('s');
		if(flag == 3)
			flag = 2;
		else 
			flag = 0;
		break;
		
	case(L_SHFT_REL): //set flag
		if(flag == 3)
			flag = 2;
		else 
			flag = 0;
		break;
	case(ALT):
		alt_flag = 1;
		break;
	case(ALT_REL):
		alt_flag = 0;
		break;
	case(F1):
		switch_terminal(sys_terminals[0]);
		break;
	case(F2):
		switch_terminal(sys_terminals[1]);
		break;
	case(F3):
		switch_terminal(sys_terminals[2]);
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
				buffer[curr_terminal][j] = NULL;
			s_code = 0x01; //clear the L scan value so it won't print
			clear();
			screen_x = 0;
			screen_y = 0; //reset the cursor and buffer
			i = 0;
		}
		else if(i < BUF_MAX)
		{
			out = scancode[flag][s_code]; //special case for L since it does not register normally
			buffer[curr_terminal][i] = out;
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
			else if(screen_x == 0) //do delete for overflows
			{
				screen_x = NUM_COLS - 2;
				screen_y--;
				putc(' ');
				screen_x = NUM_COLS - 2;
			}
			buffer[curr_terminal][i-1] = NULL;
			i--;
		update_cursor(screen_y, screen_x);		
		}
		break;
	case(ENTER):
		//do the enter implementation here
		//it will need to output bufferfer and clear it after
		buffer[curr_terminal][i] = '\n';
		i++;
		allow_read = 1;
		break;
	case(SPECIAL_KEY):
		s_code = getScancode();
		switch(s_code)
		{
			case(CTRL):
				ctrl_flag = 1;
				break;
			case(CTRL_REL):
				ctrl_flag = 0;
				break;
			default:
				break;
		}
	default:
	if(ctrl_flag == 0)
	{
		out = scancode[flag][s_code]; //fetch char from scancode
		if((out != 0) && (i < BUF_MAX))	//if valid char, print it
		{
			buffer[curr_terminal][i] = out;
			i++;
		}
		screen_x = curr_xcoord;
		screen_y = curr_ycoord; //keep track of start of buffer
		for(j = 0; j < i; j++)
		{
		/*	if(screen_x >= (NUM_COLS - 1))
			{
				if(screen_y < (NUM_ROWS - 1))	 //handle cases for scrolling and overflow
				{
					screen_y++;
					screen_x = 0;
				}
				else if (screen_y >= (NUM_ROWS - 1))
				{
					handle_max_buffer();
				}
			} */
			putc(buffer[curr_terminal][j]);
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
	if(max_terminal > 0) //decrement max terminal
	{
		if(curr_terminal == max_terminal)
			curr_terminal--; //if curr terminal is on the max, decrement it
		max_terminal--;
	}
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
	int j;
for(j = 0; j < 1; j++)
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
}
	//reset the cursor so it will be set properly
	screen_y = NUM_ROWS - 3;
	screen_x = 0;
	if(i != 0)
		curr_ycoord --;
	
//	update_cursor(screen_y, screen_x);
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
	if (max_terminal < 8)
	{
		curr_terminal++;
		max_terminal++;
	}
	int a;
	for (a = 0; a < BUF_MAX; a++)
		{buffer[curr_terminal][a] = NULL;
		}
	return 0;
}

// initialize the systems terminal structure
uint32_t init_terminals(void)
{
	int x;
	int y;
	// initialize the sys terminal struct for 3 terminals (CHECKPOINT 5)
	for(x = 0; x < NUM_TERMINAL; x ++)
	{
		sys_terminals[x].terminal_id = x;
		sys_terminals[x].read_flag = 0;
		sys_terminals[x].x_coordinate = 0;
		sys_terminals[x].y_coordinate = 0;

		for(y = 0; y < BUF_MAX; y ++)
		{
			sys_terminals[x].keyboard_buf[y] = 0;
		}

		sys_terminals[x].video_page = vir_vid + (_4KB * x);
		sys_terminals[x].storage_page = video_memory[x];
	}
	return 0;
}

// Start a new terminal and return the pointer to that terminal
terminal_struct_t *new_terminal(void)
{
	int x;
	// iterate through checking for open terminal
	for(x = 0; x < NUM_TERMINAL; x ++)
	{
		// check if there is no processes running for the given index
		if(sys_terminals[x].in_use == 0)
		{
			// if no process is running, then return address to terminal
			sys_terminals[x].in_use = 1;

			return &sys_terminals[x];
		}
	}
	// return -1 (FAIL)
	return NULL;
}

// switch to the terminal passed and switch the current process out of video memory
// and the new one into video memory
void switch_terminal(terminal_struct_t* terminal)
{
	uint32_t flags;				// local variable to save flags
	cli_and_save(flags);		// disable interrupts and save EFLAGS

	if(terminal ==  NULL)		// if the terminal passed is NULL
	{
		return;					// just return
	}

	if(map_page(cur_terminal->video_page, cur_terminal->storage_page) == -1)
	{
		return;
	}
	memcpy((void *)cur_terminal->video_page, (void *)vir_vid, _4KB);

	cur_terminal = terminal;	// the current terminal points to new terminal
	memcpy((void *) vir_vid, (void *) cur_terminal->video_page, _4KB);
	map_page(cur_terminal->video_page, VIDEO);

	restore_flags(flags);	// restore flags and reable interrupts
} 
