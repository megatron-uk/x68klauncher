/*
 bmp.c - a simple Windows BMP file loader
 based on the example at:
 https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/

 Modified for use on M68000 powered Sharp X68000 by John Snowdon (2020).
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <dos.h>

#include "utils.h"
#include "bmp.h"
#include "rgb.h"

int bmp_ReadImage(FILE *bmp_image, bmpdata_t *bmpdata, uint8_t header, uint8_t data){
	/* 
		bmp_image 	== open file handle to your bmp file
		bmpdata 	== a bmpdata_t struct
		header		== 1/0 to enable bmp header parsing
		data			== 1/0 to enable bmp pixel extraction (cannot do this without first parsing the header)
	
		Example use:
	
		FILE *f;
		bmpdata_t bmp = NULL;
		bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
		bmp->pixels = NULL;
		
		f = fopen("file.bmp", "rb");
		bmp_ReadImage(f, bmp, 1, 1);
		bmp_Destroy(bmp);
		fclose(f);
	*/
	
	uint8_t	*bmp_ptr, *bmp_ptr_old;	// Represents which row of pixels we are reading at any time
	int 		i;			// A loop counter
	int		status;		// Generic status for calls from fread/fseek etc.
	uint16_t 	pixel;		// A single pixel

	if (header){
		// Seek to dataoffset position in header
		fseek(bmp_image, 0, 1);
		status = fseek(bmp_image, DATA_OFFSET_OFFSET, SEEK_SET);
		if (status != 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error seeking data offset in header\n", __FILE__, __LINE__);
			}
			return BMP_ERR_READ;
		}
		// Read data offset value
		status = fread(&bmpdata->offset, 4, 1, bmp_image);
		if (status < 1){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error reading %d records at data offset header pos, got %d\n", __FILE__, __LINE__, 4, status);
			}
			return BMP_ERR_READ;
		}
		// offset is a little-endian 32bit, so swap it
		bmpdata->offset = swap_int32(bmpdata->offset);
		
		// Seek to image width/columns position in header
		status = fseek(bmp_image, WIDTH_OFFSET, SEEK_SET);
		if (status != 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error seeking width in header\n", __FILE__, __LINE__);
			}
			return BMP_ERR_READ;
		}
		// Read width value
		status = fread(&bmpdata->width, 4, 1, bmp_image);
		if (status < 1){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error reading %d records at image width header pos, got %d\n", __FILE__, __LINE__, 4, status);
			}
			return BMP_ERR_READ;
		}
		// width is a little-endian 32bit, so swap it
		bmpdata->width = swap_int32(bmpdata->width);
		
		// Seek to image height/rows position in header
		status = fseek(bmp_image, HEIGHT_OFFSET, SEEK_SET);
		if (status != 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error seeking height in header\n", __FILE__, __LINE__);
			}
			return BMP_ERR_READ;
		}
		// Read height value
		status = fread(&bmpdata->height, 4, 1, bmp_image);
		if (status < 1){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error reading %d records at image height header pos, got %d\n", __FILE__, __LINE__, 4, status);
			}
			return BMP_ERR_READ;
		}
		// Height is a little-endian 32bit so swap it
		bmpdata->height = swap_int32(bmpdata->height);
		
		// Seek to bpp location in header
		status = fseek(bmp_image, BITS_PER_PIXEL_OFFSET, SEEK_SET);
		if (status != 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error seeking bpp in header\n", __FILE__, __LINE__);
			}
			return BMP_ERR_READ;
		}
		// Read bpp value
		status = fread(&bmpdata->bpp, 2, 1, bmp_image);
		if (status < 1){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error reading %d records at bpp header pos, got %d\n", __FILE__, __LINE__, 2, status);
			}
			return BMP_ERR_READ;
		}
		// BPP is a little-endian 16bit, so swap it
		bmpdata->bpp = swap_int16(bmpdata->bpp);
		if ((bmpdata->bpp != BMP_8BPP) && (bmpdata->bpp != BMP_16BPP) && (bmpdata->bpp != BMP_1BPP)){
			if (BMP_VERBOSE){
				printf("%s.%d\t Unsupported pixel depth of %dbpp\n", __FILE__, __LINE__, bmpdata->bpp);
				printf("%s.%d\t The supported pixel depths are %d, %d and %d\n", __FILE__, __LINE__, BMP_8BPP, BMP_16BPP, BMP_1BPP);
			}
			return BMP_ERR_BPP;
		}
		
		// Seek to compression field
		status = fseek(bmp_image, COMPRESS_OFFSET, SEEK_SET);
		if (status != 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error seeking compression field in header\n", __FILE__, __LINE__);
			}
			return BMP_ERR_READ;
		}		
		// Read compression value
		status = fread(&bmpdata->compressed, sizeof(bmpdata->compressed), 1, bmp_image);
		if (status < 1){
			if (BMP_VERBOSE){
				printf("%s.%d\t Error reading %d records at compression type header pos, got %d\n", __FILE__, __LINE__, 4, status);
			}
			return BMP_ERR_READ;
		}
				
		// compression is a little-endian 32bit so swap it
		bmpdata->compressed = swap_int32(bmpdata->compressed);
		if (bmpdata->compressed != BMP_UNCOMPRESSED){
			if (BMP_VERBOSE){
				printf("%s.%d\t Unsupported compressed BMP format\n", __FILE__, __LINE__);
			}
			return BMP_ERR_COMPRESSED;
		}
				
		// Calculate the bytes needed to store a single pixel
		bmpdata->bytespp = (uint16_t) (bmpdata->bpp >> 3);
		
		// Rows are stored bottom-up
		// Each row is padded to be a multiple of 4 bytes. 
		// We calculate the padded row size in bytes
		if (bmpdata->bpp == BMP_1BPP){
			bmpdata->row_padded = (int)(4 * ceil((float)(bmpdata->width) / 4.0f)) / 8;
			bmpdata->row_unpadded = (int) ceil((float)bmpdata->width / 8.0f);
			bmpdata->size = (int) ceil((bmpdata->width * bmpdata->height) / 8.0f); 
			bmpdata->n_pixels = bmpdata->size;
		} else {
			bmpdata->row_padded = (int)(4 * ceil((float)(bmpdata->width) / 4.0f)) * bmpdata->bytespp; // This needs moving from ceil/floating point!!!!
			bmpdata->row_unpadded = bmpdata->width * bmpdata->bytespp;
			bmpdata->size = (bmpdata->width * bmpdata->height * bmpdata->bytespp);
			bmpdata->n_pixels = bmpdata->width * bmpdata->height;
		}
		
		if (BMP_VERBOSE){
			printf("%s.%d\t Bitmap header loaded ok!\n", __FILE__, __LINE__);
			printf("%s.%d\t Info - Resolution: %dx%d\n", __FILE__, __LINE__, bmpdata->width, bmpdata->height);
			printf("%s.%d\t Info - Padded row size: %d bytes\n", __FILE__, __LINE__, bmpdata->row_padded);
			printf("%s.%d\t Info - Unpadded row size: %d bytes\n", __FILE__, __LINE__, bmpdata->row_unpadded);
			printf("%s.%d\t Info - Colour depth: %dbpp\n", __FILE__, __LINE__, bmpdata->bpp);
			printf("%s.%d\t Info - Storage size: %d bytes\n", __FILE__, __LINE__, bmpdata->size);
			printf("%s.%d\t Info - Pixel data @ %x\n", __FILE__, __LINE__, (unsigned int) &bmpdata->pixels);
		}
	}
	
	if (data){
	
		// First verify if we've actually read the header section...
		if (bmpdata->offset <= 0){
			if (BMP_VERBOSE){
				printf("%s.%d\t Data offset not found or null, unable to seek to data section\n", __FILE__, __LINE__);
			}
			return BMP_ERR_READ;
		}
		
		// Allocate the total size of the pixel data in bytes		
		if (bmpdata->bpp == BMP_1BPP){
			bmpdata->pixels = (uint8_t*) calloc(bmpdata->size, 1);
		} else {
			bmpdata->pixels = (uint8_t*) calloc(bmpdata->n_pixels, bmpdata->bytespp);
		} 
		if (bmpdata->pixels == NULL){
			if (BMP_VERBOSE){
				printf("%s.%d\t Unable to allocate memory for pixel data\n", __FILE__, __LINE__);
			}
			return BMP_ERR_MEM;
		}
	
		// Set the pixer buffer point to point to the very end of the buffer, minus the space for one row
		// We have to read the BMP data backwards into the buffer, as it is stored in the file bottom to top
		bmp_ptr = bmpdata->pixels + ((bmpdata->height - 1) * bmpdata->row_unpadded);
		
		// Seek to start of data section in file
		fseek(bmp_image, bmpdata->offset, SEEK_SET);
		
		// For every row in the image...
		for (i = 0; i < bmpdata->height; i++){		
			
			status = fread(bmp_ptr, 1, bmpdata->row_unpadded, bmp_image);
			if (status < 1){
				if (BMP_VERBOSE){
					printf("%s.%d\t Error reading file at pos %u\n", __FILE__, __LINE__, (unsigned int) ftell(bmp_image));
					printf("%s.%d\t Error reading %d records, got %d\n", __FILE__, __LINE__, bmpdata->row_unpadded, status);
				}
				free(bmpdata->pixels);
				return BMP_ERR_READ;	
			}
			
			// Update pixel buffer position to the next row which we'll read next loop (from bottom to top)
			bmp_ptr -= bmpdata->row_unpadded;
			
			// Seek to next set of pixels for the next row if row_unpadded < row_padded
			if (status != bmpdata->row_unpadded){
				// Seek the number of bytes left in this row
				status = fseek(bmp_image, (bmpdata->row_padded - status), SEEK_CUR);
				if (status != 0){
					if (BMP_VERBOSE){
						printf("%s.%d\t Error seeking next row of pixels\n", __FILE__, __LINE__);
					}
					free(bmpdata->pixels);
					return BMP_ERR_READ;
				}
			}
			// Else... the fread() already left us at the next row	
		}		
		
		// Swap each pixel to correct endianness (this is for Sharp X68000, which is big-endian)
		// BMP pixel data is stored little-endian.
		
		// Case 1. 16bpp, use swap_int16
		if (bmpdata->bpp == BMP_16BPP){
			if (BMP_VERBOSE){
				printf("%s.%d\t Byte swapping 16bit pixels\n", __FILE__, __LINE__);
			}
			// Copy pixel buffer pointer
			bmp_ptr = bmp_ptr_old = bmpdata->pixels;
			uint8_t r,g,b;
			for(i = 0; i < bmpdata->n_pixels; i++){
				// Remember, each pixel is actually (bmpdata->bytespp) bytes
				pixel = (uint16_t) (((bmp_ptr[0] & 0xFF) << 8) | (bmp_ptr[1] & 0xFF));
				
				// Swap from the native little-endian BMP data to big-endian
				pixel = swap_int16(pixel);
				
				// Turn that 16bit pixel into 3 bytes - 5bits for red, 6 bits for green and 5 bits for blue
				// and recombine to the native GRB+I format of the X68000
				r = (((pixel & r_mask565) >> 11) << 3);
				g = (((pixel & g_mask565) >> 5) << 2); 
				b = (((pixel & b_mask565)  >> 0) << 3);
				pixel = rgb888_2grb(r, g, b, 1);
				
				bmp_ptr[0] = ((pixel & 0xFF00) >> 8);
				bmp_ptr[1] = ((pixel & 0x00FF));
				bmp_ptr += bmpdata->bytespp;
			}
			// Restore original pixel buffer pointer
			bmpdata->pixels = bmp_ptr_old;
			
		// Case 2. 8bpp
		} else if (bmpdata->bpp == BMP_8BPP){
			// We dont need to byteswap 8bpp image data
			// But we do need to extract palette information
			return BMP_OK;
			
		// Case 3. 1bpp
		} else if (bmpdata->bpp == BMP_1BPP){
			// We dont need to byteswap 1bpp image data
			return BMP_OK;
			
		// Everything else
		} else {
			if (BMP_VERBOSE){
				printf("%s.%d\t Unsupported byte mode for this pixel depth\n", __FILE__, __LINE__);
			}
			free(bmpdata->pixels);
			return BMP_ERR_BPP;
		}
	}
	return BMP_OK;
}

int bmp_ReadImageHeader(FILE *bmp_image, bmpdata_t *bmpdata){
	// Just read the header information about a BMP image into a bmpdata structure
	return bmp_ReadImage(bmp_image, bmpdata, 1, 0);	
}

int bmp_ReadImageData(FILE *bmp_image, bmpdata_t *bmpdata){
	// Just load the pixel data into an already defined bmpdata structure
	return bmp_ReadImage(bmp_image, bmpdata, 0, 1);	
}


int bmp_ReadFont(FILE *bmp_image, bmpdata_t *bmpdata, fontdata_t *fontdata, uint8_t header, uint8_t data, uint8_t font_width, uint8_t font_height){
	// Read a font from disk - really a wrapper around the bitmap reader
	int h, w;
	int bytepos;
	int heightpos;
	int pos;
	int status;
	int width_chars;
	int height_chars;
	int row_bytepos;
	char b;
	
	status = bmp_ReadImage(bmp_image, bmpdata, header, data);
	if (status == BMP_OK){
		fontdata->width = font_width;
		fontdata->height = font_height;
		if (data != 0){
			// Process BMP pixels to planar font array
			pos = 0;
			width_chars = bmpdata->width / font_width;
			height_chars = bmpdata->height / font_height;
			if (BMP_VERBOSE){
				printf("%s.%d\t Font BMP stores %d rows of %d characters (%d total symbols)\n", __FILE__, __LINE__, height_chars, width_chars, (width_chars * height_chars));	
				printf("%s.%d\t 1bpp font decoded at 0x%x\n", __FILE__, __LINE__, (unsigned int) &fontdata->symbol);
			}
			if (bmpdata->bpp == BMP_1BPP){			
				// For each WxH character in the bitmap image,
				// slice it by each row of bits and store each row as
				// a single byte for that row.
				// Since this is a 1bpp image, only store a single plane
				// of bytes, planes 1-3 are ignored.
				
				pos = 0; 			// character/symbol position (0, 1, 2, ... 32)
				bytepos = 0; 		// position in bmp image data of the top left pixel of the current symbol
				heightpos = 0;	// vertical offsetof into bmp image data
				// For every row of symbols
				for(h = 0; h < height_chars; h++){
					// FOr every symbol in the row
					for(w = 0; w < width_chars; w++){
						// For every row of pixels in a symbol
						for(heightpos = 0; heightpos < font_height; heightpos++){
							row_bytepos = bytepos + (bmpdata->row_unpadded * heightpos);
							b = bmpdata->pixels[row_bytepos];
							//printf("row %2d bytepos %3d pattern %s\n", heightpos, row_bytepos, byte2bin(b, NULL));
							fontdata->symbol[pos][heightpos][0] = b;
							fontdata->symbol[pos][heightpos][1] = 0;
							fontdata->symbol[pos][heightpos][2] = 0;
							fontdata->symbol[pos][heightpos][3] = 0;
						}
						// Jump to next symbol in row
						bytepos += 1;
						
						// Increment symbol number
						pos++;
					}
					// At end of every row of symbols, step by the height of one symbol row
					// so that we're at the top left pixel of the first symbol of the new row
					bytepos += (bmpdata->row_unpadded * (font_height - 1));
				}
				return BMP_OK;
			} else {
				// Unsupported bpp for font
				return BMP_ERR_BPP;
			}
		} else {
			return BMP_OK;
		}
	} else {
		return status;	
	}	
}

void bmp_Destroy(bmpdata_t *bmpdata){
	// Destroy a bmpdata structure and free any memory allocated
	
	if (bmpdata->pixels != NULL){
		free(bmpdata->pixels);	
	}
	free(bmpdata);	
}

void bmp_DestroyFont(fontdata_t *fontdata){
	// Destroy a fontdata structure and free any memory allocated
	
	free(fontdata);
	
}