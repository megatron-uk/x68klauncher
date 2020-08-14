/* gfxtest.c, Main executable segment for the gfx demos in x68Launcher.
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
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <iocs.h>

#include "rgb.h"
#include "gfx.h"
#include "gfxdemo.h"
#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

int main() {
	FILE *f;
	int status, super;
	uint16_t x, y, grbi;
	bmpdata_t *bmp;
	
	super = _dos_super(0);
	
	// Initialise graphics screen mode
	gfx_init(1, 1);
	
	grbi = rgb888_2grb(0xFF, 0x00, 0x00, 1);
	
	
	// ==============================
	//
	// Do a simple b/w checkerboard pattern
	//
	// ==============================
	
	printf("%s.%d\t gfx_checkerboard\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	gfx_checkerboard();
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	_iocs_wipe();
	
	
	// ==============================
	//
	// Do a 16bit colour matrix
	//
	// ==============================
	
	printf("%s.%d\t gfx_rainbow\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	gfx_rainbow();
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Vertical gradient
	//
	// ==============================
	
	printf("%s.%d\t gfx_gradient\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	gfx_gradient();
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Test converting x/y coords to vram addresses
	//
	// ==============================
	
	printf("%s.%d\t gfx XY coord test\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	gvram = (uint16_t*) gvramGetXYaddr(0, 0);
	printf("%s.%d\t x:%d y:%d gvram:%08x\n", __FILE__, __LINE__, 0, 0, (unsigned int) gvram);
	gvram = (uint16_t*) gvramGetXYaddr(511, 511);
	printf("%s.%d\t x:%d y:%d gvram:%08x\n", __FILE__, __LINE__, 511, 511, (unsigned int) gvram);
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Generate a fully red colour
	//
	// ==============================
	
	printf("%s.%d\t gfx point test\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	grbi = rgb888_2grb(0xFF, 0x00, 0x00, 1);
	gvramPoint(0, 0, grbi);
	gvramPoint(10, 10, grbi);
	gvramPoint(25, 25, grbi);
	gvramPoint(100, 100, grbi);
	gvramPoint(250, 250, grbi);
	gvramPoint(350, 350, grbi);
	gvramPoint(500, 500, grbi);
	gvramPoint(511, 511, grbi);
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Flood fill an area defined by 4 points
	//
	// ==============================
	
	printf("%s.%d\t gfx box fill test\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	gvramBoxFill(0, 0, 30, 30, grbi);
	gvramBoxFill(200, 10, 230, 300, grbi);
	grbi = rgb888_2grb(0xFF, 0xB0, 0x0F, 1);
	gvramBoxFill(300, 210, 400, 255, grbi);
	grbi = rgb888_2grb(0xF0, 0x19, 0xAF, 1);
	gvramBoxFill(489, 300, 510, 510, grbi);
	grbi = rgb888_2grb(0x00, 0x00, 0xEE, 1);
	gvramBoxFill(50, 400, 320, 500, grbi);
	grbi = rgb888_2grb(0xDD, 0xFF, 0xEE, 1);
	gvramBoxFill(0, 500, 75, 525, grbi);		// This should clip at y axis
	gvramBoxFill(1, 1, 31, 31, grbi);
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Outlined boxes
	// ==============================
	
	printf("%s.%d\t gfx box outline test\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();	
	grbi = rgb888_2grb(0xFF, 0x00, 0x00, 1);
	gvramBox(0, 50, 100, 100, grbi);
	gvramBox(1, 100, 101, 150, grbi);
	grbi = rgb888_2grb(0xFF, 0xB0, 0x0F, 1);
	gvramBox(3, 200, 103, 250, grbi);
	grbi = rgb888_2grb(0x00, 0xFF, 0x00, 1);
	gvramBox(275, 450, 500, 500, grbi);
	grbi = rgb888_2grb(0x00, 0x00, 0xEE, 1);
	gvramBox(75, 300, 200, 305, grbi);
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Bitmap display
	//
	// ==============================
	
	printf("%s.%d\t bitmap display test\n", __FILE__, __LINE__);
	printf("Press any key to start demo\n");
	_dos_getc();
	
	f = fopen("demo.bmp", "rb");
	bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	bmp->pixels = NULL;
	bmp_ReadImage(f, bmp, 1, 1);
	fclose(f);
	status = gvramBitmap(0, 0, bmp);
	bmp_Destroy(bmp);
	printf("Press any key to load next bitmap\n");
	_dos_getc();
	
	f = fopen("demo.bmp", "rb");
	bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	bmp->pixels = NULL;
	bmp_ReadImage(f, bmp, 1, 1);
	fclose(f);
	status = gvramBitmap(300, 300, bmp);
	bmp_Destroy(bmp);
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	_iocs_g_clr_on();
	
	
	// ==============================
	//
	// Bitmap clipping test
	//
	// ==============================
	
	printf("%s.%d\t bitmap clipping test\n", __FILE__, __LINE__);
	f = fopen("demo.bmp", "rb");
	bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	bmp->pixels = NULL;
	bmp_ReadImage(f, bmp, 1, 1);
	fclose(f);
	printf("Press any key to start demo\n");
	_dos_getc();	
	
	status = gvramBitmap(-20, 0, bmp);
	status = gvramBitmap(350, -70, bmp);
	status = gvramBitmap(399, 400, bmp);
	status = gvramBitmap(-150, 300, bmp);
	status = gvramBitmap(200, 200, bmp);
	bmp_Destroy(bmp);
	printf("Press any key to view next demo mode\n");
	_dos_getc();
	
	
	// ==============================
	//
	// Screen copy test
	//
	// ==============================
	
	printf("%s.%d\t screen to screen copy test\n", __FILE__, __LINE__);
	f = fopen("demo.bmp", "rb");
	bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	bmp->pixels = NULL;
	bmp_ReadImage(f, bmp, 1, 1);
	status = gvramBitmap(200, 200, bmp);
	fclose(f);
	bmp_Destroy(bmp);
	printf("Press any key to start demo\n");
	_dos_getc();	
	
	status = gvramScreenCopy(200, 200, 250, 250, 0, 0);
	status = gvramScreenCopy(200, 200, 250, 250, 0, 100);
	status = gvramScreenCopy(200, 200, 250, 250, 0, 300);
	status = gvramScreenCopy(270, 300, 350, 375, 400, 100);
	
	printf("Press any key to exit demo mode\n");
	_dos_getc();
	
	// Restore previous graphics mode
	gfx_close();
	
	// Release supervisor mode
	_dos_super(super);
	
	printf("Exited demo mode\n");
	return 0;
}