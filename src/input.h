/* input.h, User input routines for the x68Launcher.
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

#define INPUT_VERBOSE					0

// Keyboard scan groupings
#define input_group_left_right_up_down	0x07
#define input_group_select_enter			0x09
#define input_group_select_space			0x06
#define input_group_cancel				0x00
#define input_group_switch				0x02
#define input_group_misc					0x04

// Input codes as returned to main()
#define input_none						0x00
#define input_select						0x01
#define input_toggle						0x02
#define input_cancel						0x03
#define input_switch						0x04
#define input_up							0x05
#define input_down						0x06
#define input_left						0x07
#define input_right						0x08
#define input_scroll_up					0x09
#define input_scroll_down				0x0A
#define input_quit						0x0B
#define input_filter						0x0C
#define input_help						0x0D
#define input_joy_up						0x0E
#define input_joy_down					0x0F
#define input_joy_left					0x10
#define input_joy_right					0x11
#define input_joy_select					0x12
#define input_joy_cancel					0x13

// Function prototypes
int	input_get();