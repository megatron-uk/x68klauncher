/* gfx.h, GVRAM Graphical function prototypes for the x68Launcher.
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

#define GFX_VERBOSE		0				// Turn on/off gfx-specific debug output
#define GFX_PAGE		0				// Active GVRAM page (only one page in 16bit colour mode)
#define GFX_CRT_MODE 	12				// 512x512 65535 colour
#define GFX_ROWS		512				// NUmbe of pixels in a row
#define GFX_COLS		512				// Number of pixels in a column
#define GFX_ROW_SIZE	(GFX_COLS << 1)	// NUmber of bytes in a row (pixels per row * 2)
#define GFX_COL_SIZE 	(GFX_ROWS << 1)  // NUmber of bytes in a column
#define GFX_PIXEL_SIZE	2				// 2 bytes per pixel

#define GVRAM_START	0xC00000		// Start of graphics vram
#define GVRAM_END		0xC7FFFF		// End of graphics vram

#define RGB_BLACK		0x0000			// Simple RGB definition for a black 16bit pixel (5551 representation?)
#define RGB_WHITE		0xFFFF			// Simple RGB definition for a white 16bit pixel (5551 representation?)

#define GFX_ERR_UNSUPPORTED_BPP			-254
#define GFX_ERR_MISSING_BMPHEADER			-253

uint16_t	*gvram;							// Pointer to a GVRAM location (which is always as wide as a 16bit word)
int crt_last_mode;							// Store last active mode before this application runs

/* **************************** */
/* Function prototypes */
/* **************************** */
int		gfx_Init(int verbose, int clear);
int		gfx_Close();
void	gfx_Clear();
int		gvramBitmap(int x, int y, bmpdata_t *bmpdata);
int 		gvramBitmapAsync(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate);
int		gvramBox(int x1, int y1, int x2, int y2, uint16_t grbi);
int		gvramBoxFill(int x1, int y1, int x2, int y2, uint16_t grbi);
int		gvramGetXYaddr(int x, int y);
int		gvramPoint(int x, int y, uint16_t grbi);
int		gvramScreenFill(uint16_t rgb);
int		gvramScreenCopy(int x1, int y1, int x2, int y2, int x3, int y3);