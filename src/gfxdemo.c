/* gfxdemo.c, Graphical demos drawing to GVRAM for the x68Launcher.
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

#include <dos.h>
#include <iocs.h>
#include <stdio.h>
#include <limits.h>

#include "gfx.h"
#include "rgb.h"

void gfx_checkerboard(){
	int x;
	int y;
	int bit;
	uint16_t super;
	uint16_t	*gvram;
	// Set initial colour mode to white
	bit = 1;
	// Enter supervisor mode
	//super = _dos_super(0);
	// Set the initial gvram starting address
	gvram = (uint16_t*) GVRAM_START;
	// For every row
	for(y = 0; y < GFX_ROWS; y++){
		// For every column in this row
		for(x = 0; x < GFX_COLS; x++){
			if (bit){
				*gvram = RGB_WHITE;
			} else {
				*gvram = RGB_BLACK;
			}
			// Flip b/w to w/b for next column
			bit = 1 - bit;
			// Increment to next pixel location
			gvram++;
		}
		// Flip b/w to w/b for next row
		bit = 1 - bit;
	}
	// Return from supervisor mode
	//_dos_super(super);
}

void gfx_rainbow(){
	int x, y;
	uint16_t super;
	uint8_t r,g,b, i_high, i_low;
	uint16_t i, ii;
	uint16_t	*gvram;
	// Enter supervisor mode
	//super = _dos_super(0);
	
	// Set the initial counter to generate our 16bit colour index 
	i=0;
	
	// Set the initial gvram starting address
	gvram = (uint16_t*) GVRAM_START;
	
	// For every row
	for(y = 0; y < GFX_ROWS; y++){
		// For every column in this row
		for(x = 0; x < GFX_COLS; x++){
			i++; 	// Counter which generates our colour		
			if (i >= 65535){
				i = 0;
			}
			// Shift each byte left 3, we only combine the 5 msb for the 15bit+intensity value
			r = (uint8_t) (((i & r_mask555) >> 10) << 3);
			g = (uint8_t) (((i & g_mask555) >> 5) << 3);
			b = (uint8_t) (((i & b_mask555) >> 0) << 3);
			// Convert r,g,b values to x68000 grbi
			ii = rgb888_2grb(r,g,b,1);
			
			// Write single 16bit word in grb
			*gvram = ii;
			
			// If writing in 16bit word mode, step by +1
			gvram++;			
		}
	}
	// Return from supervisor mode
	//_dos_super(super);
}

void gfx_gradient(){
	int x, row;
	int y, col;
	uint16_t super;
	uint8_t r,g,b;
	uint16_t	*gvram;
	// Enter supervisor mode to access graphics memory directly
	//super = _dos_super(0);
	
	// Initialise starting colour
	r=g=b=0x00;
	
	// Starting counters for x/y position
	x = 0;
	y = 0;
	
	// Loop over every gvram pixel
	for(gvram = (uint16_t*) GVRAM_START; gvram < (uint16_t*) GVRAM_END; gvram++){
		x++;
		
		if (y < 255){
			// The upper half of the screen gets an incrementing/lightening gradient
			if (x >= 511){
				// Next line, before row 256
				y++;
				x = 0;
				if (r >= 255){
					r = g = b = 255;
				} else {
					r++;
					g++;
					b++;
				}
			}
		} else {
			// The lower half of the screen gets a decrementing/darkening gradient
			if (x >= 511){
				// Next line, after row 256
				y++;
				x = 0; 
				if (r == 0){
					r = g = b = 0;
				} else {
					r--;
					g--;
					b--;
				}
			}
		}
		// Write our 16bit colour to the current pixel
		*gvram = rgb888_2grb(r,g,b,1);
	}
	// Return from supervisor mode
	//_dos_super(super);
}