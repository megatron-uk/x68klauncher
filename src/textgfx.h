/* textgfx.h, TVRAM Graphical function prototypes for the x68Launcher.
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

#include <stdint.h>

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

#define TXT_VERBOSE		0		// Turn on/off gfx-specific debug output
#define TXT_ROWS		1024	// Number of pixels in a row
#define TXT_COLS		1024	// Number of pixels in a column
#define TXT_ROW_SIZE	64		// Number of 16bit words in a row (1024 TXT_ROWS / 16 bits in a word)

#define TVRAM_PAL_START	0xE82200

#define TVRAM_0_START	0xE00000	// Start of text vram bitplane 0
#define TVRAM_0_END		0xE1FFFF	// End of text vram bitplan 0
#define TVRAM_1_START	0xE20000	// Start of text vram bitplane 1
#define TVRAM_1_END		0xE3FFFF	// End of text vram bitplan 1
#define TVRAM_2_START	0xE40000	// Start of text vram bitplane 2
#define TVRAM_2_END		0xE5FFFF	// End of text vram bitplan 2
#define TVRAM_3_START	0xE60000	// Start of text vram bitplane 3
#define TVRAM_3_END		0xE7FFFF	// End of text vram bitplan 3

#define TVRAM_INIT_OK		0
#define TVRAM_INIT_ERR		-1

#define TVRAM_TEXT_OK		0		// Output of TVRAM data ok
#define TVRAM_TEXT_INVALID	-1		// Attempted output of an unsupported font glyph (too wide, too heigh, etc)

#define TVRAM_ADDRESS_OK	0		// Converting XY coords to TVRAM address is okay
#define TVRAM_ADDRESS_ERR	-1		// Error converting XY coords to TVRAM address

uint16_t *tvram0;
uint16_t *tvram1;
uint16_t *tvram2;
uint16_t *tvram3;

int		txt_Init();
int		txt_Close();
void	txt_Clear();
int		tvramClear8x8(int x, int y, int n_chars);
int		tvramClear8x16(int x, int y, int n_chars);
int		tvramClears(int x, int y, int char_height, int char_width, int n_chars);
int 		tvramGetXYaddr(int x, int y);
int 		tvramPutc(int x, int y, fontdata_t *fontdata, char *c);
int 		tvramPuts(int x, int y, fontdata_t *fontdata, char *c);
int		tvramPutPixels();
void		tvramSetPal(unsigned char palette, uint16_t grbi);