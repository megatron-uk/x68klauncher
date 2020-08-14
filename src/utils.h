/* utils.h, misc. helper utils prototypes for the x68Launcher.
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

// Swap endianess of a 16bit integer
#define swap_int16(i) ((i << 8) | ((i >> 8) & 0xFF))

// swap endianness of a 32bit integer
#define swap_int32(i) (((i >> 24) & 0xff) | ((i << 8) & 0xff0000) | ((i >> 8) & 0xff00) | ((i << 24) & 0xff000000))

char 		*byte2bin(uint8_t n, char *buf);
char 		*short2bin(uint16_t n, char *buf);