/* timers.c, Print performance data of two timers.
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
#include <time.h>
#include <dos.h>

#include "timers.h"

long int xclock(){
	return _dos_time_pr();
}

void timers_Print(long int start, long int end, char* name, int enabled){
	
	if (enabled){
		printf("%s.%d\t %-30s: %5ld sec\n", __FILE__, __LINE__, name, (end - start));
	}
}

int timers_FireArt(long int last){
	// Returns true if the timeout since the last input has
	// exceeded that to fire the artwork display routine
	
	long int t;
	
	return 1;
	
	/*
	t = xclock() - last;
	
	if (t >= ARTWORK_FIRE){
		return 1;
	} else {
		return 0;	
	}
	*/
}

