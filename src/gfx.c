/* gfx.c, GVRAM Graphical functions for drawing the main screen for the x68Launcher.
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
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "gfx.h"
#include "utils.h"
#include "rgb.h"
#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

int gfx_Init(){
	// Initialise graphics to a set of configured defaults
	
	if (GFX_VERBOSE){
		printf("%s.%d\t Storing previous graphics mode\n", __FILE__, __LINE__);	
	}
	crt_last_mode = _iocs_crtmod(-1);
	
	if (GFX_VERBOSE){
		printf("%s.%d\t Setting graphics mode\n", __FILE__, __LINE__);	
	}
	_iocs_crtmod(GFX_CRT_MODE);
	
	if (GFX_VERBOSE){
		printf("%s.%d\t Setting active graphics page\n", __FILE__, __LINE__);	
	}
	_iocs_vpage(GFX_PAGE);
	
	if (GFX_VERBOSE){
		printf("%s.%d\t Disable text cursor\n", __FILE__, __LINE__);	
	}
	_iocs_b_curoff();
	
	if (GFX_VERBOSE){
		printf("%s.%d\t Clearing graphics screen\n", __FILE__, __LINE__);	
	}
	_iocs_g_clr_on();
	
	return 0;
}

int gfx_Close(){
	// Release supervisor mode
	
	if (GFX_VERBOSE){
		printf("%s.%d\t Exiting gfx mode\n", __FILE__, __LINE__);	
	}
	// return previous mode??
	
	/* Restore original screen mode */
	_iocs_crtmod(crt_last_mode);
	
	/* Enable text cursor */
	_iocs_b_curon();
	return 0;
}

void gfx_Clear(){
	_iocs_b_curoff();
	_iocs_g_clr_on();
}

void gfx_Flip(){
	
}

int gvramBitmap(int x, int y, bmpdata_t *bmpdata){
	// Load bitmap data into gvram at coords x,y
	// X or Y can be negative which starts the first X or Y
	// rows or columns of the bitmap offscreen - i.e. they are clipped
	//
	// Bitmaps wider or taller than the screen are UNSUPPORTED
	
	int row, col;			//  x and y position counters
	int start_addr;		// The first pixel
	int width_bytes;		// Number of bytes in one row of the image
	int skip_cols;			// Skip first or last pixels of a row if the image is partially offscreen
	int skip_bytes;
	int skip_rows;		// Skip this number of rows if the image is patially offscreen
	int total_rows	;		// Total number of rows to read in clip mode
	uint16_t *ptr;			// Pointer to current location in pixel buffer - 16bit aligned so we always start at a pixel, and not half a byte of the last one
	
	if (x < 0){
		// Negative values start offscreen at the left
		skip_cols = x;
	} else {
		if ((x + bmpdata->width) > GFX_COLS){
			// Positive values get clipped at the right
			skip_cols = x + bmpdata->width - GFX_COLS;
		} else {
			// Full width can fit on screen
			skip_cols = 0;
		}
	}
	
	if (y < 0){
		// Negative values start off the top of the screen
		skip_rows = y;
	} else {
		if ((y + bmpdata->height) > GFX_ROWS){
			// Positive values get clipped at the bottom of the screen
			skip_rows = y + bmpdata->height - GFX_ROWS;
		} else {
			// Full height can fit on screen
			skip_rows = 0;
		}
	}
	
	if ((skip_cols == 0) && (skip_rows == 0)){
		// Case 1 - bitmap fits entirely onscreen
		width_bytes = bmpdata->width * bmpdata->bytespp;
		
		// Get starting pixel address
		start_addr = gvramGetXYaddr(x, y);
		if (start_addr < 0){
			if (GFX_VERBOSE){
				printf("%s.%d\t Unable to set GVRAM start address\n", __FILE__, __LINE__);
			}
			return -1;
		}
		// Set starting pixel address
		gvram = (uint16_t*) start_addr;
		
		// memcpy entire rows at a time
		ptr = (uint16_t*) bmpdata->pixels; // cast to 16bit 
		for(row = 0; row < bmpdata->height; row++){
			memcpy(gvram, ptr, width_bytes);
			
			// Go to next row in vram
			gvram += GFX_COLS;
			
			// Increment point
			ptr += bmpdata->width;
		}
		return 0;
		
	} else {
		
		// Case 2 - image is either vertically or horizontally partially offscreen		
		if (skip_cols < 0){
			x = x + abs(skip_cols);
		}
		if (skip_rows < 0){
			y = y + abs(skip_rows);
		}
		
		// Get starting pixel address - at the new coordinates
		start_addr = gvramGetXYaddr(x, y);
		if (start_addr < 0){
			if (GFX_VERBOSE){
				printf("%s.%d\t Unable to set GVRAM start address\n", __FILE__, __LINE__);
			}
			return -1;
		}
		// Set starting pixel address
		gvram = (uint16_t*) start_addr;
		
		// Set starting point in pixel buffer
		ptr = (uint16_t*) bmpdata->pixels; // cast to 16bit
		
		// Default to writing a full row of pixels, unless....
		width_bytes = (bmpdata->width * bmpdata->bytespp) ;
		
		// Default to writing all rows, unless....
		total_rows = bmpdata->height;
		
		// If we are starting offscreen at the y axis, jump that many rows into the data
		if (skip_rows < 0){
			ptr += abs(skip_rows) * bmpdata->width;// * bmpdata->bytespp;
			total_rows = bmpdata->height - abs(skip_rows);
		}
		if (skip_rows > 0){
			total_rows = bmpdata->height - abs(skip_rows);
		}
	
		if (skip_cols != 0){
			width_bytes = (bmpdata->width * bmpdata->bytespp) - (abs(skip_cols) * bmpdata->bytespp);
		}
		
		// memcpy entire rows at a time, subject to clipping sizes
		for(row = 0; row < total_rows; row++){
			if (skip_cols < 0){
				memcpy(gvram, ptr + abs(skip_cols), width_bytes);
			} else {
				memcpy(gvram, ptr, width_bytes);
			}
			// Go to next row in vram
			gvram += GFX_COLS;
			// Increment pointer to next row in pixel buffer
			ptr += bmpdata->width;
		}
		return 0;
	}
	
	return -1;
} 

int gvramBitmapAsync(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate){
	// Load from file, decode and display, line by line
	// Every time the function is called, another line is read, decoded and displayed
	
	int 		i;			// Loop counter
	uint16_t 	pixel;		// A single pixel
	uint8_t	*bmp_ptr;	// Access pairs of bytes in pixel bufer
	uint16_t	*ptr;		// Access 16bit words in pixel buffer
	uint8_t 	r,g,b;
	int		status;
	int 		start_addr;	// The first pixel
	int		new_y;
	
	if (bmpdata->bpp != 16){
		return GFX_ERR_UNSUPPORTED_BPP;
	}

	// BMP header has not been read yet
	if (bmpdata->offset <= 0){
		return GFX_ERR_MISSING_BMPHEADER;
	}
	
	if (bmpstate->rows_remaining == bmpdata->height){
		// This is a new image, or we haven't read a row yet
		
		//if (bmpstate->pixels != NULL){
		//	free(bmpstate->pixels);
		//}
		//bmpstate->pixels = (uint8_t*) calloc(bmpdata->width, bmpdata->bytespp);
		bmpstate->width_bytes = bmpdata->width * bmpdata->bytespp;
		
		// Seek to start of data section in file
		status = fseek(bmpfile, bmpdata->offset, SEEK_SET);
		if (status != 0){
			//free(bmpstate->pixels);
			bmpstate->width_bytes = 0;
			bmpstate->rows_remaining = 0;
			return BMP_ERR_READ;
		}
	}	
	
	// Read a row of pixels
	status = fread(bmpstate->pixels, 1, bmpdata->row_unpadded, bmpfile);
	if (status < 1){
		//free(bmpstate->pixels);
		bmpstate->width_bytes = 0;
		bmpstate->rows_remaining = 0;
		return BMP_ERR_READ;	
	}
	
	if (status != bmpdata->row_unpadded){
		// Seek the number of bytes left in this row
		status = fseek(bmpfile, (bmpdata->row_padded - bmpdata->row_unpadded), SEEK_CUR);
		if (status != 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t gfx_BitmapAsync() Error seeking next row of pixels\n", __FILE__, __LINE__);
			}
			//free(bmpstate->pixels);
			bmpstate->width_bytes = 0;
			bmpstate->rows_remaining = 0;
			return BMP_ERR_READ;
		}
	} else {
		// Seek to end of row
		if (bmpdata->row_padded != bmpdata->row_unpadded){
			fseek(bmpfile, (bmpdata->row_padded - bmpdata->row_unpadded), SEEK_CUR);
		}
	}
	
	// Get coordinates
	new_y = y + bmpstate->rows_remaining;
	start_addr = gvramGetXYaddr(x, new_y);
	
	bmp_ptr = bmpstate->pixels;
	for(i = 0; i < bmpdata->width; i++){
		// Extract a pixel
		pixel = (uint16_t) (((bmp_ptr[0] & 0xFF) << 8) | (bmp_ptr[1] & 0xFF));
		
		// Byteswap it
		pixel = swap_int16(pixel);
		
		// RGB 2 GRBI
		r = (((pixel & r_mask565) >> 11) << 3);
		g = (((pixel & g_mask565) >> 5) << 2); 
		b = (((pixel & b_mask565)  >> 0) << 3);
		pixel = rgb888_2grb(r, g, b, 1);
		
		// Store back in pixel buffer
		bmp_ptr[0] = ((pixel & 0xFF00) >> 8);
		bmp_ptr[1] = ((pixel & 0x00FF));
		bmp_ptr += bmpdata->bytespp;
	}
	
	// Set starting pixel address
	gvram = (uint16_t*) start_addr;
	
	// Copy to screen
	memcpy(gvram, bmpstate->pixels, bmpstate->width_bytes);
	
	bmpstate->rows_remaining--;
	
	if (bmpstate->rows_remaining < 1){
		bmpstate->rows_remaining = 0;
	}
	
	return 0;
	
}

int gvramBitmapAsyncFull(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate){
	// Display a bitmap using the async call, in its entirety, using no-more than 1 line
	// worth of allocated memory
	
	int status;
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gvramBitmapAsyncFull() Starting async bitmap loader\n", __FILE__, __LINE__);
	}
	
	// Read image header and palette entries
	status = bmp_ReadImage(bmpfile, bmpdata, 1, 0);	
	if (status != 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gvramBitmapAsyncFull() Unable to load bitmap for async display\n", __FILE__, __LINE__);
		}
	} else {
		if (GFX_VERBOSE){
			printf("%s.%d\t gvramBitmapAsyncFull() Bitmap header and palette loaded\n", __FILE__, __LINE__);
			printf("%s.%d\t gvramBitmapAsyncFull() %dx%d\n", __FILE__, __LINE__, bmpdata->width, bmpdata->height);
			if (bmpdata->row_unpadded != bmpdata->row_padded){
				printf("%s.%d\t gvramBitmapAsyncFull() %d / %d row size\n", __FILE__, __LINE__, bmpdata->row_unpadded, bmpdata->row_padded);
				printf("%s.%d\t gvramBitmapAsyncFull() Need to seek at the end of each row!\n", __FILE__, __LINE__);
			}
		}
		
		// Set rows remaining
		bmpstate->rows_remaining = bmpdata->height;
	
		// Loop until all rows processed
		if (GFX_VERBOSE){
			printf("%s.%d\t gvramBitmapAsyncFull() Starting async display at X:%d Y:%d...\n", __FILE__, __LINE__, x, y);
			printf("%s.%d\t gvramBitmapAsyncFull() Need to make %d calls...\n", __FILE__, __LINE__, bmpstate->rows_remaining);
		}
		while (bmpstate->rows_remaining > 0){
			status = gvramBitmapAsync(x, y, bmpdata, bmpfile, bmpstate);
			if (status != 0){
				if (GFX_VERBOSE){
					printf("%s.%d\t gvramBitmapAsyncFull() Error loading bitmap asynchronously\n", __FILE__, __LINE__);
				}
				return status;	
			}
		}
		if (GFX_VERBOSE){
			printf("%s.%d\t gvramBitmapAsyncFull() Completed\n", __FILE__, __LINE__);
		}
	}
	
	return status;
}

int gvramBox(int x1, int y1, int x2, int y2, uint16_t grbi){
	// Draw a box outline with a given grbi colour
	int row, col;		//  x and y position counters
	int start_addr;	// The first pixel, at x1,y1
	int temp;		// Holds either x or y, if we need to flip them
	int step;
	
	// Flip y, if it is supplied reversed
	if (y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	// Flip x, if it is supplied reversed
	if (x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	// Clip the x range to the edge of the screen
	if (x2>GFX_COLS){
		x2 = GFX_COLS - 1;
	}
	// Clip the y range to the bottom of the screen
	if (y2>GFX_ROWS){
		y2 = GFX_ROWS - 1;
	}
	// Get starting pixel address
	start_addr = gvramGetXYaddr(x1, y1);
	if (start_addr < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t Unable to set GVRAM start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	// Set starting pixel address
	gvram = (uint16_t*) start_addr;
	
	// Step to next row in vram
	step = (GFX_COLS - x2) + x1;
	
	// Draw top
	for(col = x1; col <= x2; col++){
		*gvram = grbi;
		// Move to next pixel in line
		gvram++;
	}
	//memset(gvram, grbi, ((x2 - x1) * 2));
	//wmemset((wchar_t *) gvram, (wchar_t) grbi, (x2 - x1));
	//gvram += (x2 - x1);
	
	// Jump to next line down and start of left side
	gvram += (GFX_COLS - x2) + (x1 - 1);
	
	// Draw sides
	for(row = y1; row < (y2-1); row++){	
		*gvram = grbi;
		gvram += (x2 - x1);
		*gvram = grbi;
		gvram += step;
	}
	
	// Draw bottom
	for(col = x1; col <= x2; col++){
		*gvram = grbi;
		// Move to next pixel in line
		gvram++;
	}
	//memset(gvram, grbi, ((x2 - x1) * 2));
	//wmemset((wchar_t *) gvram, (wchar_t) grbi, (x2 - x1));
	//gvram += (x2 - x1);
	
	return 0;
}

int gvramBoxFill(int x1, int y1, int x2, int y2, uint16_t grbi){
	// Draw a box, fill it with a given grbi colour
	int row, col;		//  x and y position counters
	int start_addr;	// The first pixel, at x1,y1
	int temp;		// Holds either x or y, if we need to flip them
	int size;
	int step;
	int stepsize;
	
	if (GFX_VERBOSE){
	   printf("%s.%d\t gfx_BoxFill() Drawing %d,%d-%d,%d\n", __FILE__, __LINE__, x1, y1, x2, y2);
	}
	
	// Flip y, if it is supplied reversed
	if (y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	// Flip x, if it is supplied reversed
	if (x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	// Clip the x range to the edge of the screen
	if (x2>GFX_COLS){
		x2 = GFX_COLS - 1;
	}
	// Clip the y range to the bottom of the screen
	if (y2>GFX_ROWS){
		y2 = GFX_ROWS - 1;
	}
	// Get starting pixel address
	start_addr = gvramGetXYaddr(x1, y1);
	if (start_addr < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t Unable to set GVRAM start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	// Set starting pixel address
	gvram = (uint16_t*) start_addr;
	
	// Step to next row in vram
	step = (GFX_COLS - x2) + (x1 - 1);
	
	// Number of bytes to write at a time (row size)
	//size = (x2 - x1);
	
	// Starting from the first row (y1)
	for(row = y1; row <= y2; row++){
		// Starting from the first column (x1)
		for(col = x1; col <= x2; col++){
			*gvram = grbi;
			gvram++;
		}
		gvram += step;
	}
	
	// Offset after each memset() call
	//stepsize = step + size + 1;
	
	// Starting from the first row (y1)
	//for(row = y1; row <= y2; row++){
	//	
	//	//memset(gvram, grbi, size * 2);
	//	wmemset((wchar_t *) gvram, (wchar_t) grbi, size);
	//	gvram += stepsize;
	//}
	
	return 0;
}

int gvramGetXYaddr(int x, int y){
	// Return the memory address of an X,Y screen coordinate based on the GFX_COLS and GFX_ROWS
	// as defined in gfx.h - if you define a different screen mode dynamically, this WILL NOT WORK
	
	uint32_t addr;
	uint16_t row;
	
	addr = GVRAM_START;
	addr += GFX_ROW_SIZE * y;
	addr += (x * GFX_PIXEL_SIZE);
	
	if (addr>GVRAM_END){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_GetXYaddr() XY coords beyond GVRAM address range\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	if (addr < GVRAM_START){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_GetXYaddr() XY coords before GVRAM address range\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	return addr;
}

int gvramPoint(int x, int y, uint16_t grbi){
	// Draw a single pixel, in a given colour at the point x,y	
	
	// Get starting pixel address
	gvram = (uint16_t*) gvramGetXYaddr(x, y);
	if (gvram < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t Unable to set GVRAM start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	*gvram = grbi;
	return 0;
}

int gvramScreenCopy(int x1, int y1, int x2, int y2, int x3, int y3){
	// Copy a block of GVRAM to another area of the screen
	// x1,y1, x2,x2	source bounding box
	// x3,y3			destination coordinates
	
	uint16_t	row;			// Row counter
	uint16_t	col;			// Column counter
	uint16_t	n_cols;		// Width of source area, in pixels
	uint16_t	n_rows;		// Height of source area, in pixels
	uint16_t *gvram_dest;	// Destination GVRAM pointer
	uint16_t	width_bytes;	// Row size, in bytes
	int start_addr;		// The first pixel in source
	int dest_addr;		// The first pixel in destination
	
	n_cols = x2 - x1;
	if (n_cols < 1){
		if (GFX_VERBOSE){
			printf("%s.%d\t Horizontal size of source must be >0\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	n_rows = x2 - x1;
	if (n_rows < 1){
		if (GFX_VERBOSE){
			printf("%s.%d\t Vertical size of source must be >0\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	// Get starting pixel address
	start_addr = gvramGetXYaddr(x1, y1);
	if (start_addr < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t Unable to set GVRAM start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	gvram = (uint16_t*) start_addr;
	
	dest_addr = gvramGetXYaddr(x3, y3);
	if (dest_addr < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t Unable to set GVRAM destination address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	gvram_dest = (uint16_t*) dest_addr;
	
	// Calculate size of copy
	width_bytes = n_cols * GFX_PIXEL_SIZE;
	
	// memcpy entire rows at a time
	for(row = 0; row < n_rows; row++){
		memcpy(gvram_dest, gvram, width_bytes);
		
		// Go to next row in vram
		gvram += GFX_COLS;
		gvram_dest += GFX_COLS;
	}
	
	return 0;
}

int gvramScreenFill(uint16_t rgb){
	// Set the entire gvram screen space to a specific rgb colour
	int c;
	uint16_t super;
	gvram = (uint16_t*) GVRAM_START;
	
	for(c = GVRAM_START; c < GVRAM_END; c++){
		*gvram = rgb;
		gvram++;
	}
	return 0;
}