/* texttest.c, TVRAM Graphical function test for the x68Launcher.
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

#include <stdlib.h>
#include <stdio.h>
#include <iocs.h>
#include <dos.h>

#include "rgb.h"
#include "gfx.h"
#include "textgfx.h"
#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

int main(){
	int			i;
	int			status;
	int 			super;
	bmpdata_t 	*font_bmp;
	fontdata_t 	*font;
	FILE			*f;
	
	super = _dos_super(0);
	
	gfx_Init(1, 1);
	txt_Init();
	
	tvramClear();
	//tvramPutPixels();
		
	font = (fontdata_t *) malloc(sizeof(fontdata_t));
	font_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	font_bmp->pixels = NULL;
	
	f = fopen("assets\\font2.bmp", "rb");
	
	status = bmp_ReadFont(f, font_bmp, font, 1, 1, 8, 16);
	font->ascii_start = 33; 		// Font table starts at ascii '!'
	font->n_symbols = 96;
	font->unknown_symbol = 95; 	// Use ascii '?' for missing symbols
	fclose(f);
	
	// Now we've converted the font bitmap to a font structure
	// we can free up the bitmap memory
	bmp_Destroy(font_bmp);
	
	tvramPuts(0, 0, font, "This font");
	tvramPuts(0, 16, font, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	tvramPuts(0, 32, font, "abcdefghijklmnopqrstuvwxyz");
	tvramPuts(0, 48, font, "!\"\'$%&@#");
	tvramPuts(0, 496, font, "is from Sonic Pinball Party!");
	
	//_dos_super(super);
	return 0;
}
