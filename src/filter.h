/* filter.h, Apply sorting and filtering to gamedata lists.
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

#ifndef __HAS_DATA
#include "data.h"
#define __HAS_DATA
#endif
#ifndef __HAS_MAIN
#include "main.h"
#define __HAS_MAIN
#endif

// Defaults
#define FILTER_VERBOSE	0		// Enable/disable logging for these functions
#define	FILTER_OK		0		// Success returncode
#define FILTER_ERR		-1		// Failure returncode

// Custom filter strings
#define FILTER_STRING_CONTROL_CYBERSTICK "Input: Cyberstick"
#define FILTER_STRING_MISC_FPU	"Misc: FPU"
#define FILTER_STRING_FLOPPY_2HDBOOT	"Floppy: 2HDBoot"
#define FILTER_STRING_FLOPPY_2HDSIM	"Floppy: 2HDSim"

// Function prototypes
int filter_GetGenres(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_GetSeries(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_GetCompany(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_GetTechSpecs(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_None(state_t *state, gamedata_t *gamedata);
int filter_Genre(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_Series(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_Company(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_TechSpecs(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
