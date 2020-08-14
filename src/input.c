/* input.c, Combined joystick and keyboar user input routines for the x68Launcher.
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

#include <iocs.h>
#include <stdio.h>

#include "input.h"

int input_get(){
	// Read joystick or keyboard input and return directions or buttons pressed
	
	int k;
	int j;
	
	// Direction cursor keys
	k = _iocs_bitsns(input_group_left_right_up_down);
	if (k & 0x08) return input_left;
	if (k & 0x20) return input_right;
	if (k & 0x10) return input_up;
	if (k & 0x40) return input_down;
	if (k & 0x01) return input_scroll_down;
	if (k & 0x02) return input_scroll_up;
	
	// Tab/switch/quit keys
	k = _iocs_bitsns(input_group_switch);
	if (k & 0x01) return input_switch;
	if (k & 0x02) return input_quit;
	
	// Enter keys
	k = _iocs_bitsns(input_group_select_enter);
	if (k & 0x40) return input_select;
	
	// Space keys (treat as enter/select)
	k = _iocs_bitsns(input_group_select_space);
	if (k & 0x20) return input_select;
	
	// Escape key (treat as cancel)
	k = _iocs_bitsns(input_group_cancel);
	if (k & 0x02) return input_cancel;
	
	// Read joystick
	j = _iocs_joyget(0);
	if (j & 0xFB) return input_left;
	if (j & 0xFB) return input_right;
	if (j & 0xFE) return input_up;
	if (j & 0xFD) return input_down;
	if (j & 0xBF) return input_select;
	if (j & 0xDF) return input_cancel;	
	
	// No input
	return input_none;
}