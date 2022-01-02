#include "screen.h"

char str[128];
static struct pos position;

static int attempt_stack[128];
int stack_idx=0;

/* Returns the screen offset for a given location */
int get_screen_offset(int col, int row){
    return (row*80+col)*2;
}

/* Returns cursor cell index from actual screen location */
uint16_t get_cursor(){
    //The device uses its control regiser as an index
    // to select its internal registers, of which we are
    //interested in:
    //  reg 14: high byte of cursors offset
    //  reg 15: low byte of cursors offset
    //Once they have been selected we can read or write a 
    //byte on the data register.
    int offset=0;
    port_byte_out(REG_SCREEN_CTRL,15);
    offset |= port_byte_in(REG_SCREEN_DATA);
    port_byte_out(REG_SCREEN_CTRL,14);
    offset |= ((uint16_t) port_byte_in(REG_SCREEN_DATA)) << 8; //as it's the high bit
    //since the cursor offset is the number of characters 
    //we multiply it by 2 to get char cell offset
    return offset*2;
}

/* Retuns pos struct for a given offset */
struct pos get_position(int offset){
    offset/=2;
    int row=offset%25;
    int col=offset-(row*80);
    position.row=row;
    position.col=col;
    return position;
}

/* Sets the cursor to a given offset on the screen */
void set_cursor(int offset){
    offset /=2; //convert from cell to char offset
    
    port_byte_out(REG_SCREEN_CTRL,14);
    port_byte_out(REG_SCREEN_DATA,(unsigned char)(offset>>8));
    port_byte_out(REG_SCREEN_CTRL,15);
    port_byte_out(REG_SCREEN_DATA,(unsigned char)(offset));
}

/* Returns new offset if scrolling is required and scrolls the screen*/
int handle_scrolling(int offset){
    //if the cursor is within the screen, return it unmodified
    if(offset < MAX_ROWS*MAX_COLS*2){ //this is probably wrong
        return offset;
    }

    //shuffle the rows back one
    int i;
    for(i=1; i<MAX_ROWS;i++){
        memcpy((char*)get_screen_offset(0,i-1)+ VIDEO_ADDRESS,
            (char*)get_screen_offset(0,i) + VIDEO_ADDRESS,
                    MAX_COLS*2);

    }
    char* last_line = (char*)get_screen_offset(0,MAX_ROWS-1) + VIDEO_ADDRESS;
    for(i=0;i<MAX_COLS*2;i++){
        last_line[i]=0;
    }
    offset-=2*MAX_COLS;

    return offset;
}

/* Prints a singular char to current cursor position */
void print_char(char character,char attribute_type){
    //Create a byte (char) pointer to the start of video memory
    unsigned char * vidmem = (unsigned char *) VIDEO_ADDRESS;

    //if attribute byte is zero, assume default style
    if(!attribute_type){
        attribute_type=WHITE_ON_BLACK;
    }

    int offset=get_cursor();
    if(character=='\n'){
        //set to last character so it wraps around when offset+2
        offset= get_screen_offset(79, offset/(2*MAX_COLS));
    }else{
        //otherwise write it to the correct bit
        vidmem[offset]=character;
        vidmem[offset+1]=attribute_type;
    }

    offset+=2;
    offset=handle_scrolling(offset);

    set_cursor(offset);

}

/* Print a char on the screen at a specific poisition or at cursor position
 * without affecting the cursor */
void print_char_loc(char character, int col, int row, char attribute_type){
    //Create a byte (char) pointer to the start of video memory
    unsigned char * vidmem = (unsigned char *) VIDEO_ADDRESS;

    //if attribute byte is zero, assume default style
    if(!attribute_type){
        attribute_type=WHITE_ON_BLACK;
    }

    //Get the video memory offset for the screen location
    int offset;

    if (col>=0 && row>=0){
        offset=get_screen_offset(col,row);
    }else{
        offset=get_cursor();
    }

    if(character=='\n'){
        //just in case this function is used in series
        offset= get_screen_offset(79, offset/(2*MAX_COLS));
    }else{
        //otherwise write it to the correct bit
        vidmem[offset]=character;
        vidmem[offset+1]=attribute_type;
    }
}

/* Calculates length of message, and recalculates offset so to end at given offset */
void print_to(char* message, int offset){
    offset= offset-(strlen(message)-1)*2;
    if(offset<0) offset=0;
    print_from(message, offset);
}

/* Prints string to display from the offset */
void print_from(char* message, int offset){
    int cursor = get_cursor();
    set_cursor(offset);
    print(message);
    set_cursor(cursor);
}

/* Writes character to the current cursor position */
void print_char_offset(char character, char attribute_type, int offset){
    unsigned char * vidmem = (unsigned char*)VIDEO_ADDRESS;
    vidmem[offset]=character;
    vidmem[offset+1]=attribute_type;
}

/* Inefficient function writing a string to a given location */
void print_at(char* message, int col, int row){
    set_cursor(get_screen_offset(col,row));
    int i=0;
    while(message[i] !=0){
        print_char(message[i++],WHITE_ON_BLACK);
    }
}

/* Basic print function without new line */
void print(char* message){
    int i=0;
    while(message[i] !=0){
        print_char(message[i++],WHITE_ON_BLACK);
    }
}
/* Basic print function with new line prepended */
void println(char* message){
    print_char('\n',WHITE_ON_BLACK);
    print(message);
}

/* Prints brackets ready for status of X */
void print_attempt(char* message){
    println("[    ] ");
    push_row();
    print(message);
}

/* Fills last brackets with OK status */
void print_ok(){
    int row=pop_row();
    print_char_loc('O',2,row,GREEN_ON_BLACK);
    print_char_loc('K',3,row,GREEN_ON_BLACK);

}

/* Fills last brackets with FAIL status */
void print_fail(){
    int row=pop_row();
    print_char_loc('F',1,row,RED_ON_BLACK);
    print_char_loc('A',2,row,RED_ON_BLACK);
    print_char_loc('I',3,row,RED_ON_BLACK);
    print_char_loc('L',4,row,RED_ON_BLACK);

}
/* Used to clear the screen */
void clear_screen(){
    int row;
    int col;

    for(row=0; row<MAX_ROWS;row++){
        for(col=0;col<MAX_COLS;col++){
            print_char_loc(' ',col,row,WHITE_ON_BLACK);
        }
    }
    set_cursor(get_screen_offset(0,0));
}

//----------Helper functions----------

void push_row(){
    attempt_stack[stack_idx]=get_cursor()/(2*MAX_COLS);
    stack_idx++;
}
int pop_row(){
    stack_idx--;
    return attempt_stack[stack_idx];
}

void test_colours(){
    int colour =0;
    for(int i=0;i<0xf;i++){
        for(int j=0;j<0xf;j++){
            print_char_loc('a',i,j,colour++);
        }
    }
}