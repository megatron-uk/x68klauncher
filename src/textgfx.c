/* textgfx.c, TVRAM Graphical functions for the x68Launcher.
 Copyright (C) 2020  John Snowdon
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <iocs.h>

#include "utils.h"
#include "textgfx.h"
#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

int TVRAM_TXT_ROWS;
int TVRAM_TXT_COLS;
int TVRAM_TXT_ROW_SIZE;

int txt_Init(){
	// This doesnt do much other than set the values of some global variables
	// based on the current crtmod video mode.
	//
	// This must be called after gfxInit() in order that we use the correct
	// screen sizes for TVRAM.
	
	int current_mode;
	
	current_mode = _iocs_crtmod(-1);
	
	switch(current_mode){
		
		case(0): // 512x512 4bit
		case(1):
			TVRAM_TXT_ROWS = TXT_ROWS / 2;
			TVRAM_TXT_COLS = TXT_COLS / 2;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 2;
			break;
		case(2): // 256x256 4bit
		case(3):
			TVRAM_TXT_ROWS = TXT_ROWS / 4;
			TVRAM_TXT_COLS = TXT_COLS / 4;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 4;
			break;
		case(4): // 512x512 4bit
		case(5):
			TVRAM_TXT_ROWS = TXT_ROWS / 2;
			TVRAM_TXT_COLS = TXT_COLS / 2;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 2;
			break;
		case(6): // 256x256 4bit
		case(7):
			TVRAM_TXT_ROWS = TXT_ROWS / 4;
			TVRAM_TXT_COLS = TXT_COLS / 4;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 4;
			break;
		case(8): // 512x512 8bit
		case(9):
			TVRAM_TXT_ROWS = TXT_ROWS / 2;
			TVRAM_TXT_COLS = TXT_COLS / 2;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 2;
			break;
		case(10): // 256x256 8bit
		case(11):
			TVRAM_TXT_ROWS = TXT_ROWS / 4;
			TVRAM_TXT_COLS = TXT_COLS / 4;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 4;
			break;
		case(12): // 512x512 16bit
		case(13):
			TVRAM_TXT_ROWS = TXT_ROWS / 2;
			TVRAM_TXT_COLS = TXT_COLS / 2;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 2;
			break;
		case(14): // 256x256 16bit
		case(15):
			TVRAM_TXT_ROWS = TXT_ROWS / 4;
			TVRAM_TXT_COLS = TXT_COLS / 4;
			TVRAM_TXT_ROW_SIZE = TXT_ROW_SIZE / 4;
			break;
		default:
			return TVRAM_INIT_ERR;
	}
	
	if (TXT_VERBOSE){
		printf("%s.%d\t TVRAM configured for %d x %d\n", __FILE__, __LINE__, TVRAM_TXT_COLS, TVRAM_TXT_ROWS);
		printf("%s.%d\t TVRAM configured for %d words per row\n", __FILE__, __LINE__, TVRAM_TXT_ROW_SIZE);
	}
	
	return TVRAM_INIT_OK;
}

int txt_Close(){
	return 0;
}

void txt_Clear(){
	int c;
	
	tvram0 = (uint16_t*) TVRAM_0_START;
	tvram1 = (uint16_t*) TVRAM_1_START;
	tvram2 = (uint16_t*) TVRAM_2_START;
	tvram3 = (uint16_t*) TVRAM_3_START;
	
	for(c = 0; tvram0 < (uint16_t*) TVRAM_0_END; c++){
		
		*tvram0 = 0x0000;
		*tvram1 = 0x0000;
		*tvram2 = 0x0000;
		*tvram3 = 0x0000;
		
		tvram0++;
		tvram1++;
		tvram2++;
		tvram3++;	
	}
}

int tvramClear8x8(int x, int y, int n_chars){
	// Clear N 8x8 characters worth of TVRAM, starting at x,y
	
	return tvramClears(x, y, 8, 8, n_chars);
}

int tvramClear8x16(int x, int y, int n_chars){
	// Clear N 8x16 characters worth of TVRAM, starting at x,y
	
	return tvramClears(x, y, 16, 8, n_chars);
}

int tvramClears(int x, int y, int char_height, int char_width, int n_chars){
	// Clear N characters 	
	
	int i;
	int start_offset;
	int font_row;
	
	start_offset = tvramGetXYaddr(x, y);
	if (start_offset < 0){
		if (TXT_VERBOSE){
			printf("%s.%d\t Unable to set TVRAM start offset\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	// 16px width chars take up twice the space of 8px wide chars
	if (char_width == 16){
		n_chars = n_chars / 2;	
	}
	
	for(i = 0; i < n_chars; i++){
		
		tvram0 = (uint16_t*) TVRAM_0_START + start_offset + i;
		tvram1 = (uint16_t*) TVRAM_1_START + start_offset + i;
		tvram2 = (uint16_t*) TVRAM_2_START + start_offset + i;
		tvram3 = (uint16_t*) TVRAM_3_START + start_offset + i;
		
		for(font_row = 0; font_row < char_height; font_row++){
			*tvram0 = 0x0000;
			*tvram1 = 0x0000;
			*tvram2 = 0x0000;
			*tvram3 = 0x0000;
			tvram0 += 64;
			tvram1 += 64;
			tvram2 += 64;
			tvram3 += 64;
		}	
	}
	
	return TVRAM_TEXT_OK;
}

int tvramGetXYaddr(int x, int y){
	
	// Although there are 1024x1024 pixels in TVRAM screen, 
	// these are broken down into 16bit words, so you really
	// only have 1024 / 16 = 64 x 64 addressable locations unless
	// you start getting in to applying variable bitmasks on existing
	// 8bit addressable regions... lets... just ... not.
	
	// This means 
	// 0 <= x <= TVRAM_TXT_ROW_SIZE
	// 0 <= y <= TVRAM_TXT_ROWS
	
	// This implementation is a little different to gvramGetXYaddr(), as
	// we return an offset from the start address, rather than an absolute
	// address - the reason being is that we mostly likely want to apply
	// that offset to several TVRAM pages, each of which have a different
	// starting address.
	
	uint32_t	offset;
	uint8_t	row;
	offset = 0;
	
	if (x >= TVRAM_TXT_ROW_SIZE){
		if (TXT_VERBOSE){
			printf("%s.%d\t X coord beyond TVRAM final column\n", __FILE__, __LINE__);
		}
		return -1;	
	}
	
	if (y >= TXT_ROWS){
		if (TXT_VERBOSE){
			printf("%s.%d\t Y coord beyond TVRAM final row\n", __FILE__, __LINE__);
		}
		return -1;	
	}
	
	offset = TXT_ROW_SIZE * y;
	offset += x;
	
	if ((TVRAM_0_START + offset) > TVRAM_0_END){
		if (TXT_VERBOSE){
			printf("%s.%d\t XY coords beyond TVRAM address range\n", __FILE__, __LINE__);
		}
		return -1;
	}
	return offset;
}


int tvramPutc(int x, int y, fontdata_t *fontdata, char *c){
	// Place a single ascii character on the screen, at column x, row y,
	// using the font defined in fontdata_t
	
	int		start_offset;
	uint8_t	font_symbol;
	uint8_t	font_row;
	uint8_t	i;
	uint8_t	shift_places;
	
	font_row = 0;
		
	// Get starting address of TVRAM, based on the x/y row/column coordinates
	start_offset = tvramGetXYaddr(x, y);
	if (start_offset < 0){
		if (TXT_VERBOSE){
			printf("%s.%d\t Unable to set TVRAM start offset\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	// Reposition write position
	tvram0 = (uint16_t*) TVRAM_0_START + start_offset;
	tvram1 = (uint16_t*) TVRAM_1_START + start_offset;
	tvram2 = (uint16_t*) TVRAM_2_START + start_offset;
	tvram3 = (uint16_t*) TVRAM_3_START + start_offset;
	
	i = (int) c[0];
	
	// Check that this character is in our font table
	if ((i >= fontdata->ascii_start) && (i <= (fontdata->ascii_start + fontdata->n_symbols))){
		font_symbol = i - fontdata->ascii_start;
	} else {
		if (TXT_VERBOSE){
			printf("%s.%d\t Missing ASCII symbol #%d\n", __FILE__, __LINE__, i);	
		}
		font_symbol = fontdata->unknown_symbol;
	}
	
	// Left shift the symbol if its not a full width font, so that it is left-justified	
	if (fontdata->width == 8){
		shift_places = 8;	
	} else if (fontdata->width == 16){
		shift_places = 0;	
	} else {
		// Unsupported font width
		return TVRAM_TEXT_INVALID;	
	}
		
	// Draw each line of pixels in the font to every plane (as it may be in colour)
	for(font_row = 0; font_row < fontdata->height; font_row++){
		*tvram0 = fontdata->symbol[font_symbol][font_row][0] << shift_places;
		tvram0 += TXT_ROW_SIZE;
		*tvram1 = fontdata->symbol[font_symbol][font_row][1] << shift_places;
		tvram1 += TXT_ROW_SIZE;
		*tvram2 = fontdata->symbol[font_symbol][font_row][2] << shift_places;
		tvram2 += TXT_ROW_SIZE;
		*tvram3 = fontdata->symbol[font_symbol][font_row][3] << shift_places;
		tvram3 += TXT_ROW_SIZE;
	}
	return TVRAM_TEXT_OK;
}

int tvramPutPixels(){
	// Set a 16x16 block of pixels in Text-VRAM
	
	uint8_t	row;
	
	tvram0 = (uint16_t*) TVRAM_0_START;
	tvram1 = (uint16_t*) TVRAM_1_START;
	tvram2 = (uint16_t*) TVRAM_2_START;
	tvram3 = (uint16_t*) TVRAM_3_START;
	
	for(row = 0; row < 16; row++){
		*tvram0 = 0x0000;
		*tvram1 = 0x0000;
		*tvram2 = 0x0000;
		*tvram3 = 0x0000;	
		
		tvram0 += 64;
		tvram1 += 64;
		tvram2 += 64;
		tvram3 += 64;
	}
	
	return TVRAM_TEXT_OK;
}

int tvramPuts(int x, int y, fontdata_t *fontdata, char *c){
	// Put a string of text on the screen, at a set of coordinates
	// using a specific font.
	//
	// Note: We only support 8px and 16px wide fonts.
	
	int		start_offset;
	int		next_offset;
	uint8_t	font_symbol_l, font_symbol_r;
	uint8_t	font_row;
	uint8_t	font_plane;
	uint8_t	i;
	uint8_t	pos;
	uint8_t	has_right;
	uint8_t	symbol_left_0, symbol_right_0;
	uint8_t	symbol_left_1, symbol_right_1;
	uint8_t	symbol_left_2, symbol_right_2;
	uint8_t	symbol_left_3, symbol_right_3;
	uint16_t	symbol_0;
	uint16_t	symbol_1;
	uint16_t	symbol_2;
	uint16_t	symbol_3;
	
	// Empty string
	if (strlen(c) < 1){
		return TVRAM_TEXT_OK;
	}
	
	// Calculate starting address
	start_offset = tvramGetXYaddr(x, y);
	if (start_offset < 0){
		if (TXT_VERBOSE){
			printf("%s.%d\t Unable to set TVRAM start offset\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	// Reposition write position
	tvram0 = (uint16_t*) TVRAM_0_START + start_offset;
	tvram1 = (uint16_t*) TVRAM_1_START + start_offset;
	tvram2 = (uint16_t*) TVRAM_2_START + start_offset;
	tvram3 = (uint16_t*) TVRAM_3_START + start_offset;
	
	if (fontdata->width == 8){
		// For every pair of symbols in the string,
		// 1. Look up the appropriate symbol number to ascii character
		// 2. Check if the symbol is in our font table
		// 3. For the each symbol of each pair
		// 3a. Set the upper byte of a 16bit word to this symbol (left shift by 8)
		// 3b. Set the lower byte of a 16bit word to the even symbol
		// 4. Write to each plane
		
		for (pos = 0; pos < strlen(c); pos+=2){
			
			// Is there another character at pos+1
			if ((pos + 1) < strlen(c)){
				has_right = 1;	
			} else {
				has_right = 0;	
			}
			
			// Get left symbol
			i = (uint8_t) c[pos];
			if ((i >= fontdata->ascii_start) && (i <= (fontdata->ascii_start + fontdata->n_symbols))){
				font_symbol_l = i - fontdata->ascii_start;
			} else {
				font_symbol_l = fontdata->unknown_symbol;
			}

			if (has_right){
				// Get right symbol
				i = (uint8_t) c[pos+1];
				if ((i >= fontdata->ascii_start) && (i <= (fontdata->ascii_start + fontdata->n_symbols))){
					font_symbol_r = i - fontdata->ascii_start;
				} else {
					font_symbol_r = fontdata->unknown_symbol;
				}
			
				// Left and right symbols need to be merged
				for (font_row = 0; font_row < fontdata->height; font_row++){
					*tvram0 = (fontdata->symbol[font_symbol_l][font_row][0] << 8) | fontdata->symbol[font_symbol_r][font_row][0];
					tvram0 += TXT_ROW_SIZE;
					*tvram1 = (fontdata->symbol[font_symbol_l][font_row][1] << 8) | fontdata->symbol[font_symbol_r][font_row][1];
					tvram1 += TXT_ROW_SIZE;
					*tvram2 = (fontdata->symbol[font_symbol_l][font_row][2] << 8) | fontdata->symbol[font_symbol_r][font_row][2];
					tvram2 += TXT_ROW_SIZE;
					*tvram3 = (fontdata->symbol[font_symbol_l][font_row][3] << 8) | fontdata->symbol[font_symbol_r][font_row][3];
					tvram3 += TXT_ROW_SIZE;
				}
			} else {
				// Just this symbol left in string
				for(font_row = 0; font_row < fontdata->height; font_row++){
					*tvram0 = fontdata->symbol[font_symbol_l][font_row][0] << 8;
					tvram0 += TXT_ROW_SIZE;
					*tvram1 = fontdata->symbol[font_symbol_l][font_row][1] << 8;
					tvram1 += TXT_ROW_SIZE;
					*tvram2 = fontdata->symbol[font_symbol_l][font_row][2] << 8;
					tvram2 += TXT_ROW_SIZE;
					*tvram3 = fontdata->symbol[font_symbol_l][font_row][3] << 8;
					tvram3 += TXT_ROW_SIZE;
				}				
			}
			
			// Reposition write position for next 16 pixel wide symbol
			next_offset = start_offset += 1;
			tvram0 = (uint16_t*) TVRAM_0_START + next_offset;
			tvram1 = (uint16_t*) TVRAM_1_START + next_offset;
			tvram2 = (uint16_t*) TVRAM_2_START + next_offset;
			tvram3 = (uint16_t*) TVRAM_3_START + next_offset;
		}
		
		return TVRAM_TEXT_OK;
		
	} else if (fontdata->width == 16){
		// For every symbol in the string,
		// 1. Look up the appropriate symbol number to ascii character
		// 2. Check if the symbol is in our font table
		// 3a. Write font->height rows of single 16bit words to tvram
		// 3b. Write a placeholder to tvam for the character we dont have
		for (pos = 0; pos < strlen(c); pos+=2){
			i = (uint8_t) c[pos];
			if ((i >= fontdata->ascii_start) && (i <= (fontdata->ascii_start + fontdata->n_symbols))){
				font_symbol_l = i - fontdata->ascii_start;
			} else {
				font_symbol_l = fontdata->unknown_symbol;
			}
			
			// Just this symbol left in string
			for(font_row = 0; font_row < fontdata->height; font_row++){
				*tvram0 = fontdata->symbol[font_symbol_l][font_row][0] << 8;
				tvram0 += TXT_ROW_SIZE;
				*tvram1 = fontdata->symbol[font_symbol_l][font_row][1] << 8;
				tvram1 += TXT_ROW_SIZE;
				*tvram2 = fontdata->symbol[font_symbol_l][font_row][2] << 8;
				tvram2 += TXT_ROW_SIZE;
				*tvram3 = fontdata->symbol[font_symbol_l][font_row][3] << 8;
				tvram3 += TXT_ROW_SIZE;
			}
		
			// Reposition write position for next 16 pixel wide symbol
			next_offset = start_offset += 1;
			tvram0 = (uint16_t*) TVRAM_0_START + next_offset;
			tvram1 = (uint16_t*) TVRAM_1_START + next_offset;
			tvram2 = (uint16_t*) TVRAM_2_START + next_offset;
			tvram3 = (uint16_t*) TVRAM_3_START + next_offset;
		}
		return TVRAM_TEXT_OK;
		
	} else {
		// Unsupported font width
		return TVRAM_TEXT_INVALID;
	}
	
}