#pragma once

#include "int.h"
#include "../drivers/low_level.h"
#include "string.h"
#include "typedefs.h"

#define VIDEO_ADDRESS 0xC00b8000
#define MAX_ROWS 25
#define MAX_COLS 80

#define WHITE_ON_BLACK 0x0f
#define WHITE_ON_BLUE  0x1f
#define GREEN_ON_BLACK 0x0a
#define RED_ON_BLACK   0x0c


//Screen device IO ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5


extern char str[128];

struct pos{
    int row;
    int col;
};


int get_screen_offset(int col, int row);
uint16_t get_cursor();
struct pos get_position(int offset);
void set_cursor(int offset);
int handle_scrolling(int offset);
void print_char_loc(char character, int col, int row, char attribute_type);
void print_at(char* message, int col, int row);
void print(char* message);
void println(char* message);
void print_attempt(char* message);
void print_ok();
void print_fail();
void clear_screen();
void test_colours();