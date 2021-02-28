/* ui.c, Draws the user interface for the x68Launcher.
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
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#include "data.h"
#include "textgfx.h"
#include "ui.h"
#include "rgb.h" 

#ifndef __HAS_GFX
#include "gfx.h"
#define __HAS_GFX
#endif

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

// bmpdata_t structures are needed permanently for all ui 
// bitmap elements, as we may need to repaint the screen at
// periodic intervals after having dialogue boxes or menus open.
bmpdata_t 	*ui_checkbox_bmp;
bmpdata_t 	*ui_checkbox_choose_bmp;
bmpdata_t 	*ui_checkbox_empty_bmp;
bmpdata_t 	*ui_border_left_bmp;
bmpdata_t 	*ui_border_right_bmp;
bmpdata_t 	*ui_border_divider_bmp;
bmpdata_t 	*ui_header_bmp;
bmpdata_t	*ui_under_artwork_bmp;
bmpdata_t	*ui_under_browser_bmp;
bmpdata_t 	*ui_textbox_left_bmp;
bmpdata_t 	*ui_textbox_mid_bmp;
bmpdata_t	*ui_textbox_right_bmp;
bmpdata_t 	*ui_select_bmp;
bmpdata_t	*ui_font_bmp;		// Generic, just used during loading each font and then freed

// We should only need one file handle, as we'll load all of the ui
// bitmap assets sequentially.... just remember to close it at the 
// end of each function!
FILE 		*ui_asset_reader;

// Fonts
fontdata_t 	*ui_progress_font;
fontdata_t 	*ui_status_font;

// Global variable to indicate asset load status
static int 	ui_fonts_status;
static int 	ui_assets_status;

void ui_Init(){
	// Set the basic palette entries for all the user interface elements
	// NOT including any bitmaps we load - just the basic colours
	
	// Mix some colours into grbi
	PALETTE_UI_BLACK = rgb888_2grb(0, 0, 0, 0);
	PALETTE_UI_WHITE = rgb888_2grb(255, 255, 255, 0);
	PALETTE_UI_LGREY = rgb888_2grb(180, 180, 180, 0);
	PALETTE_UI_MGREY = rgb888_2grb(90, 90, 90, 0);
	PALETTE_UI_DGREY = rgb888_2grb(30, 30, 30, 0);
	PALETTE_UI_RED = rgb888_2grb(220, 0, 0, 0);
	PALETTE_UI_GREEN = rgb888_2grb(0, 220, 0, 0);
	PALETTE_UI_BLUE = rgb888_2grb(70, 123, 212, 1);
	PALETTE_UI_YELLOW = rgb888_2grb(180, 220, 20, 0);
	
	// Set palette entries for the text ram
	tvramSetPal(PALETTE_UI_WHITE_TEXT, PALETTE_UI_WHITE);
}

void ui_Close(){
	if (ui_assets_status == UI_ASSETS_LOADED){
		bmp_Destroy(ui_checkbox_bmp);
		bmp_Destroy(ui_checkbox_choose_bmp);
		bmp_Destroy(ui_checkbox_empty_bmp);
		bmp_Destroy(ui_under_artwork_bmp);
		bmp_Destroy(ui_under_browser_bmp);
		bmp_Destroy(ui_border_left_bmp);
		bmp_Destroy(ui_border_right_bmp);
		bmp_Destroy(ui_border_divider_bmp);
		bmp_Destroy(ui_header_bmp);
		bmp_Destroy(ui_textbox_left_bmp);
		bmp_Destroy(ui_textbox_mid_bmp);
		bmp_Destroy(ui_textbox_right_bmp);
		bmp_Destroy(ui_font_bmp);
		bmp_Destroy(ui_select_bmp);
	}
}

int ui_DisplayArtwork(FILE *screenshot_file, bmpdata_t *screenshot_bmp, bmpstate_t *screenshot_state, state_t *state, imagefile_t *imagefile){

	int status;
	char msg[64];
	
	// Restart artwork display
	// =======================
	// Close previous screenshot file handle
	// =======================
	if (screenshot_file != NULL){
		fclose(screenshot_file);
		screenshot_file = NULL;
	}
	
	// Clear artwork window
	gvramBoxFill(ui_artwork_xpos, ui_artwork_ypos, ui_artwork_xpos + ui_artwork_width, ui_artwork_ypos + ui_artwork_height, PALETTE_UI_BLACK);
	
		if (state->has_images){
		
		// Construct full path of image
		sprintf(msg, "%s\\%s", state->selected_game->path, imagefile->filename[imagefile->selected]);
		strcpy(state->selected_image, msg);
		if (UI_VERBOSE){
			printf("%s.%d\t ui_DisplayArtwork() Selected artwork [%d] filename [%s]\n", __FILE__, __LINE__, imagefile->selected, imagefile->filename[imagefile->selected]);
		}
		
		// =======================
		// Open new screenshot file, ready parse
		// =======================
		if (UI_VERBOSE){
			printf("%s.%d\t ui_DisplayArtwork() Opening artwork file\n", __FILE__, __LINE__);	
		}
		screenshot_file = fopen(state->selected_image, "rb");
		if (screenshot_file == NULL){
			if (UI_VERBOSE){
				printf("%s.%d\t ui_DisplayArtwork() Error, unable to open artwork file %s\n", __FILE__, __LINE__, state->selected_image);	
			}
		} 
		else {
			// =======================
			// Load header of screenshot bmp
			// =======================
			if (UI_VERBOSE){
				printf("%s.%d\t ui_DisplayArtwork() Reading BMP header\n", __FILE__, __LINE__);	
			}
		
			status = bmp_ReadImage(screenshot_file, screenshot_bmp, 1, 0);
			if (status == BMP_OK){
				screenshot_state->rows_remaining = screenshot_bmp->height;
				status = gvramBitmapAsyncFull(ui_artwork_xpos + ((ui_artwork_width - screenshot_bmp->width) / 2) , ui_artwork_ypos + ((ui_artwork_height - screenshot_bmp->height) / 2), screenshot_bmp, ui_asset_reader, screenshot_state);
			}
		}
		if (UI_VERBOSE){
			printf("%s.%d\t ui_DisplayArtwork() Call to display %s complete\n", __FILE__, __LINE__, imagefile->filename[imagefile->selected]);	
		}
		if (screenshot_file != NULL){
			fclose(screenshot_file);
			screenshot_file = NULL;
		}
	}
	return UI_OK;
}

int ui_DrawFilterPrePopup(state_t *state, int toggle){
	// Draw a popup that allows the user to toggle filter mode between genre, series and off
	unsigned char i;
	
	// Draw drop-shadow
	//gvramBoxFillTranslucent(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 10, ui_launch_popup_xpos + 10 + ui_launch_popup_width, ui_launch_popup_ypos + 10 + ui_launch_popup_height + 30, PALETTE_UI_DGREY);
	
	// Draw main box
	gvramBoxFill(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height + 90, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gvramBox(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height + 90, PALETTE_UI_LGREY);
	
	// Clear background text
	for (i = 0; i < 12; i++){
		tvramClear8x16(6, ui_launch_popup_ypos + (i * 16), 60);
	}
	
	// Box title
	tvramPuts(11, ui_launch_popup_ypos + 10, ui_progress_font, "Enable Filter?");
	// No filter text
	tvramPuts(9, ui_launch_popup_ypos + 39, ui_progress_font, "No filter - Show all games");
	// Genre filter text
	tvramPuts(9, ui_launch_popup_ypos + 69, ui_progress_font, "By Genre");
	// Series filter text
	tvramPuts(9, ui_launch_popup_ypos + 99, ui_progress_font, "By Series");
	// Copmany filter text
	tvramPuts(9, ui_launch_popup_ypos + 129, ui_progress_font, "By Company");
	// Tech specs filter text
	tvramPuts(9, ui_launch_popup_ypos + 159, ui_progress_font, "By Technical Specs");
	
	// Toggle which entry is selected
	if (toggle == 1){
		state->selected_filter++;	
	} 
	if (toggle == -1){
		state->selected_filter--;	
	}
	
	// Detect wraparound
	if (state->selected_filter >= FILTER_MAX){
		state->selected_filter = FILTER_MAX;
	}
	if (state->selected_filter < 1){
		state->selected_filter = FILTER_NONE;
	}
	
	if (state->selected_filter == FILTER_NONE){
		// none
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_GENRE){
		// genre
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_SERIES){
		// series
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_COMPANY){
		// developer/company/publisher
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_TECH){
		// developer/company/publisher
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_bmp);
	}
	
	return UI_OK;
}

int	ui_DrawConfirmPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat){
	// Draw a confirmation box to start the game
	
	// Draw drop-shadow
	//gvramBoxFillTranslucent(ui_launch_popup_xpos + 60, ui_launch_popup_ypos - 30, ui_launch_popup_xpos + 260, ui_launch_popup_ypos + 50, PALETTE_UI_DGREY);
	
	// Draw main box
	gvramBoxFill(ui_launch_popup_xpos + 50, ui_launch_popup_ypos - 40, ui_launch_popup_xpos + 250, ui_launch_popup_ypos + 40, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gvramBox(ui_launch_popup_xpos + 50, ui_launch_popup_ypos - 40, ui_launch_popup_xpos + 250, ui_launch_popup_ypos + 40, PALETTE_UI_LGREY);
	
	tvramPuts(ui_launch_popup_xpos + 110, ui_launch_popup_ypos - 30, ui_progress_font, "Start Game?");
	
	tvramPuts(ui_launch_popup_xpos + 60, ui_launch_popup_ypos - 5, ui_progress_font, "Confirm (Enter)");
	tvramPuts(ui_launch_popup_xpos + 60, ui_launch_popup_ypos + 15, ui_progress_font, "Cancel (Esc)");
	
	
	return UI_OK;
}

int ui_DrawFilterPopup(state_t *state, int select, int redraw, int toggle){
	// Draw a page of filter choices for the user to select
	// The 'redraw' parameter controls whether the text and background should be redrawn
	// on this page... if just scrolling between choices, only the 
	// checkbox bitmap is redrawn, all of the text and background can
	// be left as-is, and hence speed up the interface redraw.
	// The 'toggle' parameter controls whether we move the selection bitmap or not.
	
	int offset;
	int min_selection;
	int max_selection;
	int i;
	int page_i;
	int status;
	char msg[64]; // Title
	char buf[32];
	
	// Draw drop-shadow
	//gfx_BoxFillTranslucent(40, 50, GFX_COLS - 30, GFX_ROWS - 20, PALETTE_UI_DGREY);
	
	// The minimum and maximum index of the filter strings we can select
	min_selection = 0;
	max_selection = 0;
	
	if (redraw == 0){
		// Only paint the selection window if this is an entirely new window or new page of filters
		// Draw main box
		gvramBoxFill(30, 40, GFX_COLS - 40, GFX_ROWS - 40, PALETTE_UI_BLACK);
		// Draw main box outline
		gvramBox(30, 40, GFX_COLS - 40, GFX_ROWS - 40, PALETTE_UI_LGREY);
		
		// Clear background text
		for (i = 0; i < 27; i++){
			tvramClear8x16(1, 40 + (i * 16), 60);
		}
	}
	
	
	if (state->selected_filter == FILTER_TECH){
		// Tech specs filtering uses a multi-choice interface
		ui_DrawMultiChoiceFilterPopup(state, select, redraw, toggle);
	} else {
		
		// All other single-choice filters		
		// Only print the Window title if we are painting an entirely new window
		if (redraw == 0){
			if (state->selected_filter == FILTER_GENRE){
				sprintf(msg, "Select Genre - Page %d/%d - Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
				tvramPuts(5, 45, ui_progress_font, msg);
			}
			if (state->selected_filter == FILTER_SERIES){
				sprintf(msg, "Select Series - Page %d/%d - Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
				tvramPuts(5, 45, ui_progress_font, msg);
			}
			if (state->selected_filter == FILTER_COMPANY){
				sprintf(msg, "Select Company - Page %d/%d - Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
				tvramPuts(5, 45, ui_progress_font, msg);
			}
		}
		
		// Move the selection through the on-screen choices
		if (select == -1){
			state->selected_filter_string--;
		}
		if (select == 1){
			state->selected_filter_string++;
		}
			
		// We only draw filter string choices from
		// the currently selected page of filter strings,
		// which is held in the state->current_filter_page variable.
		offset = state->current_filter_page * MAXIMUM_FILTER_STRINGS_PER_PAGE;
		min_selection = offset;
		
		// Are there more slots on this page than there are total strings?
		if (min_selection + MAXIMUM_FILTER_STRINGS_PER_PAGE > state->available_filter_strings){
			// Upper limit is the total number of strings
			max_selection = state->available_filter_strings;
		} else {
			// Upper limit is the offset plus the total strings on this page
			max_selection = 	offset + MAXIMUM_FILTER_STRINGS_PER_PAGE;
		}
		 
		// Dont allow cursor to go below first string on page
		if (state->selected_filter_string < min_selection){
			state->selected_filter_string = min_selection;
		}
		// Dont allow cursor to go above last string on page
		if (state->selected_filter_string >= max_selection){
			state->selected_filter_string = max_selection - 1;
		}
		
		// Loop through and print the list of choices on this page, highlighting the currently
		// selected choice.
		for(i=offset; i<MAXIMUM_FILTER_STRINGS; i++){
			
			// We may (probably are) be starting part way
			// into the list of filter strings if we are on page > 1.
			page_i = i - offset;
			
			if ((state->filter_strings[i] != NULL) && (strcmp(state->filter_strings[i], "") != 0)){
			
				// Column 1
				if (page_i < MAXIMUM_FILTER_STRINGS_PER_COL){
					if (i == state->selected_filter_string){
						// This is selected
						gvramBitmap(38, 70 + (page_i * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gvramBitmap(38, 70 + (page_i * 25), ui_checkbox_empty_bmp);
					}
					// Only print the text if we are painting an entirely new window
					if (redraw == 0){
						if (strlen(state->filter_strings[i]) > 22){
							sprintf(buf, "%.20s..", state->filter_strings[i]);
							tvramPuts(4, 74 + (page_i * 25), ui_progress_font, buf);
						} else {
							tvramPuts(4, 74 + (page_i * 25), ui_progress_font, state->filter_strings[i]);
						}
					}
				}
				
				// Column 2
				if ((page_i >= MAXIMUM_FILTER_STRINGS_PER_COL) && (page_i < (2 * MAXIMUM_FILTER_STRINGS_PER_COL))){
					if (i == state->selected_filter_string){
						// This is selected
						gvramBitmap(245, 70 + ((page_i - MAXIMUM_FILTER_STRINGS_PER_COL) * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gvramBitmap(245, 70 + ((page_i - MAXIMUM_FILTER_STRINGS_PER_COL) * 25), ui_checkbox_empty_bmp);
					}
					// Only print the text if we are painting an entirely new window
					if (redraw == 0){
						if (strlen(state->filter_strings[i]) > 22){
							sprintf(buf, "%.20s..", state->filter_strings[i]);
							tvramPuts(17, 74 + (page_i * MAXIMUM_FILTER_STRINGS_PER_COL), ui_progress_font, buf);
						} else {
							tvramPuts(17, 74 + ((page_i - MAXIMUM_FILTER_STRINGS_PER_COL) * 25), ui_progress_font, state->filter_strings[i]);
						}
					}
				}
			}
		}
	}
	return UI_OK;
	
}

int ui_DrawMultiChoiceFilterPopup(state_t *state, int select, int redraw, int toggle){
	// Draw a multi-choice filter window - more than one filter can be selected at the
	// same time.
	
	int offset;
	int min_selection;
	int max_selection;
	int i;
	int page_i;
	int status;
	char msg[128]; // Title
	
	if (redraw == 0){
		
		// Clear background text
		for (i = 0; i < 27; i++){
			tvramClear8x16(1, 40 + (i * 16), 60);
		}
		
		// Title for tech specs multi-choice filter
		if (state->selected_filter == FILTER_TECH){
			// Window title
			sprintf(msg, "Select Tech Specs - Page %d/%d - Space to toggle, Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
			tvramPuts(60, 45, ui_progress_font, msg);
		}
		// Any other multi-choice filter...
		// ...
	}
	
	// Move the selection through the on-screen choices
	if (select == -1){
		state->selected_filter_string--;
	}
	if (select == 1){
		state->selected_filter_string++;
	}
		
	// We only draw filter string choices from
	// the currently selected page of filter strings,
	// which is held in the state->current_filter_page variable.
	offset = state->current_filter_page * MAXIMUM_FILTER_STRINGS_PER_PAGE;
	min_selection = offset;
	
	// Are there more slots on this page than there are total strings?
	if (min_selection + MAXIMUM_FILTER_STRINGS_PER_PAGE > state->available_filter_strings){
		// Upper limit is the total number of strings
		max_selection = state->available_filter_strings;
	} else {
		// Upper limit is the offset plus the total strings on this page
		max_selection = 	offset + MAXIMUM_FILTER_STRINGS_PER_PAGE;
	}
	
	// Dont allow cursor to go below first string on page
	if (state->selected_filter_string < min_selection){
		state->selected_filter_string = min_selection;
	}
	// Dont allow cursor to go above last string on page
	if (state->selected_filter_string >= max_selection){
		state->selected_filter_string = max_selection - 1;
	}
	
	// Loop through and print the list of choices on this page, highlighting the currently
	// selected choice.
	for(i=offset; i<MAXIMUM_FILTER_STRINGS; i++){
		
		// We may (probably are) be starting part way
		// into the list of filter strings if we are on page > 1.
		page_i = i - offset;
		
		if ((state->filter_strings[i] != NULL) && (strcmp(state->filter_strings[i], "") != 0)){
		
			// Column 1
			if (page_i < MAXIMUM_FILTER_STRINGS_PER_COL){
				// Show the selection cursor
				if (i == state->selected_filter_string){
					gvramBitmap(38, 70 + (page_i * 25), ui_checkbox_choose_bmp);
				} else {
					if (state->filter_strings_selected[i] == 1){
						// This is selected, show the checkbox
						gvramBitmap(38, 70 + (page_i * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gvramBitmap(38, 70 + (page_i * 25), ui_checkbox_empty_bmp);
					}
				}
				// Only print the text if we are painting an entirely new window
				if (redraw == 0){
					tvramPuts(4, 70 + (page_i * 25), ui_progress_font, state->filter_strings[i]);
				}
			}
			
			// Column 2
			if ((page_i >= MAXIMUM_FILTER_STRINGS_PER_COL) && (page_i < (2 * MAXIMUM_FILTER_STRINGS_PER_COL))){
				// Show the selection cursor
				if (i == state->selected_filter_string){
					gvramBitmap(225, 70 + ((page_i - 11) * 25), ui_checkbox_choose_bmp);
				} else {
					if (state->filter_strings_selected[i] == 1){
						// This is selected
						gvramBitmap(225, 70 + ((page_i - 11) * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gvramBitmap(225, 70 + ((page_i - 11) * 25), ui_checkbox_empty_bmp);
					}
				}
				// Only print the text if we are painting an entirely new window
				if (redraw == 0){
					tvramPuts(20, 70 + ((page_i - 11) * 25), ui_progress_font, state->filter_strings[i]);
				}
			}
		}
	}
	
	return UI_OK;
}

int ui_DrawHelpPopup(){
	// Display the full-screen help text	
	
	// Draw main box
	gvramBoxFill(30, 20, GFX_COLS - 40, GFX_ROWS - 20, PALETTE_UI_BLACK);
	// Draw main box outline
	gvramBox(30, 20, GFX_COLS - 40, GFX_ROWS - 20, PALETTE_UI_LGREY);
	
	tvramPuts(240, 25, ui_progress_font, "x68kLauncher - Help");
	
	// Key help
	tvramPuts(40, 45, ui_progress_font, "Key controls:");
	tvramPuts(40, 65, ui_progress_font, "- [F]      Bring up the game search/filter window");
	tvramPuts(40, 85, ui_progress_font, "- [H]      Show this help text window");
	tvramPuts(40, 105, ui_progress_font, "- [Q]      Quit the application");
	tvramPuts(40, 125, ui_progress_font, "- [Space]  Select a filter in a multi-select filter window");
	tvramPuts(40, 145, ui_progress_font, "- [Enter]  Confirm a filter choice or launch selected gameq");
	tvramPuts(40, 165, ui_progress_font, "- [Esc]    Close the current window or Cancel a selection");
	
	// Filter help
	tvramPuts(40, 200, ui_progress_font, "Search/Filter:");
	tvramPuts(40, 220, ui_progress_font, "You can search your list of games by [Genre], [Series], [Company] or");
	tvramPuts(40, 240, ui_progress_font, "by selecting one or more [Tech Specs] such as specific sound or audio");
	tvramPuts(40, 260, ui_progress_font, "device. Your games must have metadata [launch.dat] for this to work.");
	
	// Launching help
	tvramPuts(40, 295, ui_progress_font, "Game Browser:");
	tvramPuts(40, 315, ui_progress_font, "[Up] & [Down] scrolls through the list of games on a page. [PageUp]");
	tvramPuts(40, 335, ui_progress_font, "& [PageDown] jumps an entire page at a time. [Enter] launches the");
	tvramPuts(40, 355, ui_progress_font, "currently selected game. [Left] & [Right] scrolls through artwork.");
	
	return UI_OK;
}

int	ui_DrawLaunchPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat, int toggle){
	// Draw the popup window that lets us select from the main or alternate start file
	// in order to launch a game
	
	int status;	
	
	// Draw drop-shadow
	//gfx_BoxFillTranslucent(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 10, ui_launch_popup_xpos + 10 + ui_launch_popup_width, ui_launch_popup_ypos + 10 + ui_launch_popup_height, PALETTE_UI_DGREY);
	
	// Draw main box
	gvramBoxFill(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gvramBox(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height, PALETTE_UI_LGREY);
	
	tvramPuts(ui_launch_popup_xpos + 50, ui_launch_popup_ypos + 10, ui_progress_font, "Select which file to run:");
	
	// Start file text
	tvramPuts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 35, ui_progress_font, launchdat->start);
	
	// Alt start file text
	tvramPuts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 65, ui_progress_font, launchdat->alt_start);
	
	if (toggle == 1){
		state->selected_start = !state->selected_start;	
	}
	
	if (state->selected_start == 0){
		// Checkbox for start
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);		
	} else {
		// Checkbox for alt_start
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gvramBitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_bmp);
	}
	
	return UI_OK;
}

int ui_DrawInfoBox(){
	// Draw the main info pane at the bottom of the screen, including placeholders
	// for the various names, genres, etc.
	// Lets have some nice graphics/buttons, too.
	int 			status;
	
	// Draw bitmap for 
	// Game ID

	// Real Name
	ui_DrawTextPanel(ui_info_name_xpos, ui_info_name_ypos, ui_info_name_width);
	
	// Year
	ui_DrawTextPanel(ui_info_year_xpos, ui_info_year_ypos, ui_info_year_width);
	
	// Company
	ui_DrawTextPanel(ui_info_company_xpos, ui_info_company_ypos, ui_info_company_width);
	
	// Genre
	ui_DrawTextPanel(ui_info_genre_xpos, ui_info_genre_ypos, ui_info_genre_width);
	
	// Path
	ui_DrawTextPanel(ui_info_path_xpos, ui_info_path_ypos, ui_info_path_width);
	
	// Bitmap checkboxes
	// Has metadata
	gvramBitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_empty_bmp);
	
	// Has Start file?
	gvramBitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
	
	// Has alternate start file?
	gvramBitmap(ui_checkbox_has_metadata_startbat_xpos, ui_checkbox_has_metadata_startbat_ypos, ui_checkbox_empty_bmp);
	
	// Has images?
	gvramBitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);	
	
	return UI_OK;
}

int ui_DrawMainWindow(){
	
	int	x, y;
	int 	status;
	bmpdata_t 	*bmp;
	
	// Header
	// Left Border
	// Divider
	// Right Border
	// Under artwork
	// Under browser
	
	gfx_Clear();
	txt_Clear();
	
	// Header
	gvramBitmap(ui_header_xpos, ui_header_ypos, ui_header_bmp);
	
	// Left border
	gvramBitmap(ui_border_left_xpos, ui_border_left_ypos, ui_border_left_bmp);
	
	// Middle divider
	gvramBitmap(ui_border_central_xpos, ui_border_central_ypos, ui_border_divider_bmp);
	
	// Right Border
	gvramBitmap(ui_border_right_xpos, ui_border_right_ypos, ui_border_right_bmp);
	
	// Under artwork
	gvramBitmap(ui_under_artwork_xpos, ui_under_artwork_ypos, ui_under_artwork_bmp);
	
	// Under browser
	gvramBitmap(ui_under_browser_xpos, ui_under_browser_ypos, ui_under_browser_bmp);
	
	return UI_OK;
}

int ui_DrawStatusBar(){
	/* 
		Draw a small status bar at the bottom of the screen where various 
		one-liner messages can be printed.
	*/
	
	// 1px border around the main box
	
	gvramBox(
		ui_status_bar_xpos, 
		ui_status_bar_ypos, 
		ui_status_bar_xpos + ui_status_bar_width, 
		ui_status_bar_ypos + ui_status_bar_height, 
		PALETTE_UI_MGREY
	);	
	
	// Inner box, where text goes
	
	gvramBoxFill(
		ui_status_bar_xpos + 1, 
		ui_status_bar_ypos  + 1,
		ui_status_bar_xpos + ui_status_bar_width - 1,
		ui_status_bar_ypos + ui_status_bar_height - 1,
		PALETTE_UI_BLACK
	);
	
	// Progress bar drawn okay
	return UI_OK;
}

int ui_DrawSplash(){
	/*
		Show the initial splash screen which is shown during the loading
		process whilst bmp assets are loaded into ram, the disk is scanned
		for games and other stuff before the user can access the main menu.
	*/
	int			status;
	bmpdata_t 	*logo_bmp;
	
	// Load splash logo
	ui_asset_reader = fopen(splash_logo, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	logo_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	logo_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, logo_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	gvramBitmap((GFX_COLS / 2) - (logo_bmp->width / 2), (GFX_ROWS / 2) - (logo_bmp->height / 2), logo_bmp);
	
	// Destroy in-memory bitmap
	bmp_Destroy(logo_bmp);
	
	// Splash loaded okay
	return UI_OK;
	
}

int ui_DrawSplashProgress(int redraw, int progress_width){
	/*
		Display a progress bar (at 'percent') on the splash screen.
	*/
	
	int 	status;
	
	// Redraw entire graphic
	if (redraw){
		status = gvramBox(
			splash_progress_x_pos, 
			splash_progress_y_pos, 
			splash_progress_x_pos + splash_progress_width, 
			splash_progress_y_pos + splash_progress_height, 
			PALETTE_UI_LGREY
		);	
		if (status != 0){
			return UI_ERR_FUNCTION_CALL;
		}
	}
	
	// Update progress bar to 'percent'
	//(splash_progress_x_pos + 10), 
	status = gvramBoxFill(
		splash_progress_x_pos + 2, 
		(splash_progress_y_pos + 2),
		(splash_progress_x_pos + 2 + abs(progress_width)),
		(splash_progress_y_pos + (splash_progress_height - 2)),
		PALETTE_UI_BLUE
	);
	if (status != 0){
		return UI_ERR_FUNCTION_CALL;
	}
	
	// Progress bar drawn okay
	return UI_OK;
}

int ui_DrawTextPanel(int x, int y, int width){
	// Draws a text entry panel of a specific length	
	
	int i;
	int mid_width;
	
	// Calculate width of middle section of text panel
	mid_width = width - (ui_textbox_left_bmp->width + ui_textbox_right_bmp->width);
		
	// Left bitmap
	gvramBitmap(x, y, ui_textbox_left_bmp);

	// Middle bitmap(s)
	for (i = 0; i < mid_width; i+= ui_textbox_mid_bmp->width){
		gvramBitmap((x + ui_textbox_left_bmp->width + i), y, ui_textbox_mid_bmp);
	}
	
	// right bitmap
	gvramBitmap((x + mid_width), y, ui_textbox_right_bmp);
	
	return UI_OK;
}

int ui_LoadAssets(){
	/*
	   Loads all user interface bitmap assets into global bmpdata_t structures
	   
	   Uses the global ui_asset_reader file handle - only one instance open
	   at any given point.
	
	   Returns -1 on file load error or bitmap data read error
	   Returns 0 on success of all assets being loaded
	*/
	
	int status;
	
	// Default to assets not loaded
	ui_assets_status = UI_ASSETS_MISSING;
	
	// Select
	ui_ProgressMessage("Loading select icon...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_select);
	}
	ui_asset_reader = fopen(ui_select, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load select icon file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_select_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_select_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_select_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for select icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for select icon");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Checkbox
	ui_ProgressMessage("Loading checkbox icon...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_check_box);
	}
	ui_asset_reader = fopen(ui_check_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load checkbox icon file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_checkbox_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_checkbox_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for checkbox icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for checkbox icon");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Checkbox - empty
	ui_ProgressMessage("Loading checkbox (empty) icon...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_check_box_unchecked);
	}
	ui_asset_reader = fopen(ui_check_box_unchecked, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load checkbox (empty) icon file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_checkbox_empty_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_checkbox_empty_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_empty_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for checkbox (empty) icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for checkbox (empty) icon");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Checkbox choose icon
	ui_ProgressMessage("Loading checkbox (select) icon...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_check_box_choose);
	}
	ui_asset_reader = fopen(ui_check_box_choose, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load checkbox (select) icon file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_checkbox_choose_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_checkbox_choose_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_choose_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for checkbox (select) icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for checkbox (select) icon");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Textbox fragments
	ui_ProgressMessage("Loading textbox (left border)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_textbox_left);
	}
	ui_asset_reader = fopen(ui_textbox_left, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load textbox (left border) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_textbox_left_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_textbox_left_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_textbox_left_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for textbox (left border).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for textbox (left border)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Textbox fragments
	ui_ProgressMessage("Loading textbox (mid section)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_textbox_mid);
	}
	ui_asset_reader = fopen(ui_textbox_mid, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load textbox (mid section) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_textbox_mid_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_textbox_mid_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_textbox_mid_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for textbox (mid section).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for textbox (mid section)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Textbox fragments
	ui_ProgressMessage("Loading textbox (right border)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_textbox_right);
	}
	ui_asset_reader = fopen(ui_textbox_right, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load textbox (right border) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_textbox_right_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_textbox_right_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_textbox_right_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for textbox (left border).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for textbox (left border)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	
	// Header
	ui_ProgressMessage("Loading borders (header)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_header);
	}
	ui_asset_reader = fopen(ui_header, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load borders (header) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_header_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_header_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_header_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for border (header).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for border (header)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Left border
	ui_ProgressMessage("Loading borders (left)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_border_left);
	}
	ui_asset_reader = fopen(ui_border_left, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load borders (left) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_border_left_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_border_left_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_border_left_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for border (left).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for border (left)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Middle divider
	ui_ProgressMessage("Loading borders (middle)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_border_central);
	}
	ui_asset_reader = fopen(ui_border_central, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load borders (middle) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_border_divider_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_border_divider_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_border_divider_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for border (middle).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for border (middle)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Right Border
	ui_ProgressMessage("Loading borders (right)...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_border_right);
	}
	ui_asset_reader = fopen(ui_border_right, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load borders (right) file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_border_right_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_border_right_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_border_right_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for border (right).\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for border (right)");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Under artwork
	ui_ProgressMessage("Loading artwork bitmap...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_under_artwork);
	}
	ui_asset_reader = fopen(ui_under_artwork, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load artwork bitmap file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_under_artwork_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_under_artwork_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_under_artwork_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for artwork bitmap.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for artwork bitmap");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);

	// Under browser
	ui_ProgressMessage("Loading browser bitmap...");
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_under_browser);
	}
	ui_asset_reader = fopen(ui_under_browser, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to load browser bitmap file");
		_dos_getchar();
		return UI_ERR_FILE;	
	}
	ui_under_browser_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_under_browser_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_under_browser_bmp, 1, 1);
	if (status != 0){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for artwork bitmap.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for artwork bitmap");
		_dos_getchar();
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);

	// Set assets loaded status
	ui_assets_status = UI_ASSETS_LOADED;
	return UI_OK;
}

int ui_LoadFonts(){
	/*
	   Load all required fonts from disk, process them, assign to fontdata_t
	   structures in memory.
	*/
	
	int status;
	
	// Default to assets not loaded
	ui_fonts_status = UI_ASSETS_MISSING;
	
	// =========================
	// progress bar font
	// =========================
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadFonts() Loading %s\n", __FILE__, __LINE__, ui_progress_font_name);
	}
	ui_asset_reader = fopen(ui_progress_font_name, "rb");
	if (ui_asset_reader == NULL){
		if (UI_VERBOSE){
			printf("%s.%d\t Error loading UI font data\n", __FILE__, __LINE__);
		}
		return UI_ERR_FILE;	
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadFonts() Processing font data for %s\n", __FILE__, __LINE__, ui_progress_font_name);
	}
	ui_progress_font = (fontdata_t *) malloc(sizeof(fontdata_t));
	ui_font_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_font_bmp->pixels = NULL;
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_progress_font, 1, 1, ui_progress_font_width, ui_progress_font_height);
	if (status != 0){
		if (UI_VERBOSE){
			printf("%s.%d\t Error processing UI font data\n", __FILE__, __LINE__);
		}
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	ui_progress_font->ascii_start = ui_progress_font_ascii_start; 		
	ui_progress_font->n_symbols = ui_progress_font_total_syms;
	ui_progress_font->unknown_symbol = ui_progress_font_unknown;
	bmp_Destroy(ui_font_bmp);
	
	// =========================
	// status bar font
	// =========================
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadFonts() Loading %s\n", __FILE__, __LINE__, ui_progress_font_name);
	}
	ui_asset_reader = fopen(ui_status_font_name, "rb");
	if (ui_asset_reader == NULL){
		if (UI_VERBOSE){
			printf("%s.%d\t Error loading UI font data\n", __FILE__, __LINE__);
		}
		return UI_ERR_FILE;	
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadFonts() Processing font data for %s\n", __FILE__, __LINE__, ui_progress_font_name);
	}
	ui_status_font = (fontdata_t *) malloc(sizeof(fontdata_t));
	ui_font_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_font_bmp->pixels = NULL;
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_status_font, 1, 1, ui_status_font_width, ui_status_font_height);
	if (status != 0){
		if (UI_VERBOSE){
			printf("%s.%d\t Error processing UI font data\n", __FILE__, __LINE__);
		}
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	ui_status_font->ascii_start = ui_status_font_ascii_start; 		
	ui_status_font->n_symbols = ui_status_font_total_syms;
	ui_status_font->unknown_symbol = ui_status_font_unknown;
	
	// Set fonts loaded status
	ui_fonts_status = UI_ASSETS_LOADED;
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_LoadFonts() All fonts loaded\n", __FILE__, __LINE__);
	}
	
	bmp_Destroy(ui_font_bmp);
	return UI_OK;
}

int ui_ProgressMessage(char *c){
	// Output a message in the space above the progress bar

	// Clear the progress message row
	tvramPuts(ui_progress_font_x_pos, ui_progress_font_y_pos, ui_progress_font, "                                             ");
	
	// Print the text
	return tvramPuts(ui_progress_font_x_pos, ui_progress_font_y_pos, ui_progress_font, c);
}

int ui_ReselectCurrentGame(state_t *state){
	// Simply updates the selected_gameid with whatever line / page is currently selected
	// Should be called every time up/down/pageup/pagedown is detected whilst in browser pane
	
	int			startpos;		// Index of first displayable element of state->selected_items
	int			endpos;		// Index to last displayable element of state->selected_items
	int 			i;			// Loop counter
	int			selected;		// Counter to match the selected_line number
	int			gameid;		// ID of the current game we are iterating through in the selected_list
	
	// Don't allow startpos to go negative
	startpos = (state->selected_page - 1) * ui_browser_max_lines;
	if (startpos < 0){
		startpos = 0;	
	}
	
	// If we're on the last page, then make sure we only loop over the number of entries
	// that are on this page... not just all 22
	if ((startpos + ui_browser_max_lines) > state->selected_max){
		endpos = state->selected_max;
	} else {
		endpos = startpos + ui_browser_max_lines;
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_ReselectCurrentGame() Game: %d, endpos: %d, selected line: %d\n", __FILE__, __LINE__, startpos, endpos, state->selected_line);
	}
	
	selected = 0;
	for(i = startpos; i < endpos ; i++){
		gameid = state->selected_list[i];
		
		// This is the current selected game
		if (selected == state->selected_line){
			state->selected_gameid = gameid;
			return UI_OK;
		}	
		selected++;
	}
	
	return UI_OK;
}

int ui_StatusMessage(char *c){
	// Output a status message in the status bar at the bottom of the screen in the main UI

	// Clear the progress message row
	tvramPuts(ui_status_font_x_pos, ui_status_font_y_pos, ui_status_font, "                                                     ");
	
	return tvramPuts(ui_status_font_x_pos, ui_status_font_y_pos, ui_status_font, c);
}

int ui_SwitchPane(state_t *state){
	int new_pane;
		
	ui_StatusMessage("??? pane selected");
	
	// back to first pane
	if (state->active_pane == PANE_MAX){
		state->active_pane = PANE_FIRST;
	} else {
		state->active_pane += 1;	
	}
	
	if (state->active_pane == BROWSER_PANE){
		ui_StatusMessage("Browser pane selected");	
	}
	
	if (state->active_pane == ARTWORK_PANE){
		ui_StatusMessage("Artwork pane selected");	
	}
	
	if (state->active_pane == INFO_PANE){
		ui_StatusMessage("Info pane selected");	
	}
	
	// De-highlight all panes
	// TO DO
	
	// Highlight title of selected pane
	// TO DO
	
	return UI_OK;
}

int ui_UpdateBrowserPane(state_t *state, gamedata_t *gamedata){
	// UPdate the contents of the game browser pane

	// selected_list : contains the gameids that are in the currently filtered selection (e.g. ALL, shooter genre only, only by Konami, etc)
	// selected_max : is the count of how many games are in the current selection
	// selected_page : is the page (browser list can show 0 - x items per page) into the selected_list
	// selected_line : is the line of the selected_page that is highlighted
	
	gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	int			y;				// Vertical position offset for each row
	int 			i;				// Loop counter
	int 			gameid;			// ID of each game in selected_list
	char			msg[64];			// Message buffer for each row
	int			startpos;		// Index of first displayable element of state->selected_items
	int			endpos;			// Index to last displayable element of state->selected_items
	
	// Don't allow startpos to go negative
	startpos = (state->selected_page - 1) * ui_browser_max_lines;
	if (startpos < 0){
		startpos = 0;	
	}
	
	// If we're on the last page, then make sure we only draw the number of lines
	// that are on this page... not just all 22
	if ((startpos + ui_browser_max_lines) > state->selected_max){
		endpos = state->selected_max;
	} else {
		endpos = startpos + ui_browser_max_lines;
	}
	
	// Simple black to clear selection icon
	gvramBoxFill(ui_browser_cursor_xpos, ui_browser_font_y_pos, ui_browser_cursor_xpos + ui_select_bmp->width, ui_header_bmp->height + ui_border_left_bmp->height - 2, PALETTE_UI_BLACK);
	
	// Clear all lines of text
	y = ui_browser_font_y_pos;
	for(i = 0; i <= ui_browser_max_lines; i++){
		tvramClear8x16(ui_browser_font_x_pos, y, 25);
		y += ui_progress_font->height + 2;
	}
	
	// Display the entries for this page
	gamedata_head = gamedata;
	y = ui_browser_font_y_pos;
	if (UI_VERBOSE){
		printf("%s.%d\t ui_UpdateBrowserPane() Building browser menu [%d-%d]\n", __FILE__, __LINE__, startpos, endpos);
	}
	for(i = startpos; i < endpos ; i++){
		gamedata = gamedata_head;
		gameid = state->selected_list[i];
		selected_game = getGameid(gameid, gamedata);
		if (UI_VERBOSE){
			printf("%s.%d\t ui_UpdateBrowserPane() - Line %d: Game ID %d, %s\n", __FILE__, __LINE__, i, gameid, selected_game->name);
		}
		if (strlen(selected_game->name) > 24){
			sprintf(msg, "%.22s..", selected_game->name);
		} else {
			sprintf(msg, "%s", selected_game->name);
		}
		tvramPuts(ui_browser_font_x_pos + 1, y, ui_progress_font, msg);
		y += ui_progress_font->height + 2;
	}
	gamedata = gamedata_head;
	
	return UI_OK;
}

int ui_UpdateBrowserPaneStatus(state_t *state){
	// Draw browser pane status message in status panel
	char	msg[64];		// Message buffer for the status bar
	int y_pos;
	
	// Insert selection cursor
	if (state->selected_line == 0){
		y_pos = 0;
	} else {
		y_pos = (ui_progress_font->height + 2 ) * (state->selected_line);
	}
	if (UI_VERBOSE){
		printf("%s.%d\t ui_UpdateBrowserPaneStatus() Drawing selection icon at line %d, x:%d y:%d\n", __FILE__, __LINE__, state->selected_line, ui_browser_cursor_xpos, (ui_browser_font_y_pos + y_pos));
	}
	// Simple black to clear selection icon
	gvramBoxFill(ui_browser_cursor_xpos, ui_browser_font_y_pos, ui_browser_cursor_xpos + ui_select_bmp->width, ui_header_bmp->height + ui_border_left_bmp->height - 2, PALETTE_UI_BLACK);
	gvramBitmap(ui_browser_cursor_xpos, ui_browser_font_y_pos + y_pos, ui_select_bmp);
	
	// Text at bottom of browser pane
	sprintf(msg, "Line %02d/%02d     Page %02d/%02d", state->selected_line, ui_browser_max_lines - 1, state->selected_page, state->total_pages);
	tvramPuts(ui_browser_footer_font_xpos, ui_browser_footer_font_ypos, ui_status_font, msg);
	
	return UI_OK;
}

int ui_UpdateInfoPane(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat){
	// Draw the contents of the info panel with current selected game, current filter mode, etc
	
	// TO DO
	// Clear text on load
	// snprintf instead of sprintf to limit string sizes
	
	int			status;
	char		status_msg[64];		// Message buffer for anything needing to be printed onscreen
	char		info_name[64];
	char		info_year[8];
	char		info_company[32];
	char		info_path[64];
	char		info_genre[16];
	char		s1, s2;
	
	gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	//launchdat_t	*launchdat;		// Metadata object representing the launch.dat file for this game
	
	// Store gamedata head
	gamedata_head = gamedata;
	
	// Find the game data entry for this ID
	//selected_game = getGameid(state->selected_gameid, gamedata);
	
	// Clear all existing text
	tvramClear8x16(ui_info_name_text_xpos, ui_info_name_text_ypos, 30);
	tvramClear8x16(ui_info_year_text_xpos, ui_info_year_text_ypos, 8);
	tvramClear8x16(ui_info_company_text_xpos, ui_info_company_text_ypos, 26);
	tvramClear8x16(ui_info_genre_text_xpos, ui_info_genre_text_ypos, 16);
	tvramClear8x16(ui_info_path_text_xpos, ui_info_path_text_ypos, 48);
	
	// See if it has a launch.dat metadata file
	if (state->selected_game != NULL){
		if (state->selected_game->has_dat != 0){
			if (launchdat == NULL){
				// ======================
				// Unable to load launch.dat	 from disk
				// ======================
				sprintf(status_msg, "ERROR: Unable to load metadata file: %s\%s", selected_game->path, GAMEDAT);
				gvramBitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_bmp);
				gvramBitmap(ui_checkbox_has_metadata_startbat_xpos, ui_checkbox_has_metadata_startbat_ypos, ui_checkbox_empty_bmp);
				gvramBitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
				sprintf(info_name, " %s", state->selected_game->name);
				sprintf(info_year, "N/A");
				sprintf(info_company, " N/A");
				sprintf(info_genre, "N/A");
				sprintf(info_path, " %s", state->selected_game->path);
			} else {
				// ======================
				// Loaded launch.dat from disk
				// ======================
				gvramBitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_bmp);
				
				if (state->has_images == 1){
					gvramBitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_bmp);
				} else {
					gvramBitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);	
				}
				
				if (launchdat->realname != NULL){
					if (strlen(launchdat->realname) > 30){
						sprintf(info_name, " %.28s..", launchdat->realname);
					} else {
						sprintf(info_name, " %.30s", launchdat->realname);
					}
				} else {
					sprintf(info_name, " %s", state->selected_game->name);
				}
				
				if (launchdat->genre != NULL){
					sprintf(info_genre, "%.10s", launchdat->genre);
				} else {
					sprintf(info_genre, "N/A");
				}
				
				if (launchdat->year != 0){
					sprintf(info_year, "%4d", launchdat->year);
				} else {
					sprintf(info_year, "N/A");
				}
				
				s1 = strlen(launchdat->developer);
				s2 = strlen(launchdat->publisher);
				
				// If we have publisher and developer, print both
				if ((s1 > 0) && (s2 > 0)){
					
					if (s1 + s2 <= 25){
						sprintf(info_company, " %s/%s", launchdat->developer, launchdat->publisher);
					} else {
						if (s1 >= 16){
							// If developer is long, then prioritise it
							if (s2 > 12){
								sprintf(info_company, " %.15s/%.9s..", launchdat->developer, launchdat->publisher);
							} else {
								sprintf(info_company, " %.15s/%s", launchdat->developer, launchdat->publisher);
							}
						} else {
							// Otherwise just print both strings truncated to 16 places
							sprintf(info_company, " %.12s/%.12s", launchdat->developer, launchdat->publisher);
						}
					}
					
				// If we just have developer, print that
				} else if (strlen(launchdat->developer) > 0){
					sprintf(info_company, " %.25s", launchdat->developer);
					
				// If we just have publisher, print that
				} else if (strlen(launchdat->publisher) > 0){
					sprintf(info_company, " %.25s", launchdat->publisher);
					
				// If we have nothing...
				} else {
					sprintf(info_company, " N/A");
				}
				
				// Number of images/screenshots
				
				// Path
				sprintf(info_path, " %s", state->selected_game->path);
				
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - game id #1: (%d)\n", __FILE__, __LINE__, state->selected_game->gameid);
					printf("%s.%d\t ui_UpdateInfoPane()  - game id #2: (%d)\n", __FILE__, __LINE__, state->selected_gameid);
					printf("%s.%d\t ui_UpdateInfoPane()  - name: (%s)\n", __FILE__, __LINE__, info_name);
					printf("%s.%d\t ui_UpdateInfoPane()  - genre: (%s)\n", __FILE__, __LINE__, info_genre);
					printf("%s.%d\t ui_UpdateInfoPane()  - year: (%s)\n", __FILE__, __LINE__, info_year);
					printf("%s.%d\t ui_UpdateInfoPane()  - company: (%s)\n", __FILE__, __LINE__, info_company);
					printf("%s.%d\t ui_UpdateInfoPane()  - path: (%s)\n", __FILE__, __LINE__, info_path);
					printf("%s.%d\t ui_UpdateInfoPane()  - has_images: (%d)\n", __FILE__, __LINE__, state->has_images);
				}
			}
			free(launchdat);
		} else {
			// ======================
			// We can only use the basic directory information
			// ======================
			gvramBitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_empty_bmp);
			gvramBitmap(ui_checkbox_has_metadata_startbat_xpos, ui_checkbox_has_metadata_startbat_ypos, ui_checkbox_empty_bmp);
			gvramBitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
			sprintf(info_name, " %s", state->selected_game->name);
			sprintf(info_year, "N/A");
			sprintf(info_company, " N/A");
			sprintf(info_genre, "N/A");
			sprintf(info_path, " %s", state->selected_game->path);
			if (UI_VERBOSE){
				printf("%s.%d\t ui_UpdateInfoPane()  - game id #1: (%d)\n", __FILE__, __LINE__, state->selected_game->gameid);
				printf("%s.%d\t ui_UpdateInfoPane()  - game id #2: (%d)\n", __FILE__, __LINE__, state->selected_gameid);
				printf("%s.%d\t ui_UpdateInfoPane()  - name: (%s)\n", __FILE__, __LINE__, info_name);
				printf("%s.%d\t ui_UpdateInfoPane()  - genre: (%s)\n", __FILE__, __LINE__, info_genre);
				printf("%s.%d\t ui_UpdateInfoPane()  - year: (%s)\n", __FILE__, __LINE__, info_year);
				printf("%s.%d\t ui_UpdateInfoPane()  - company: (%s)\n", __FILE__, __LINE__, info_company);
				printf("%s.%d\t ui_UpdateInfoPane()  - path: (%s)\n", __FILE__, __LINE__, info_path);
				printf("%s.%d\t ui_UpdateInfoPane()  - has_images: (%d)\n", __FILE__, __LINE__, state->has_images);
			}
		}	
	} else {
		// ======================
		// Error in logic, gameid is not set
		// ======================
		sprintf(status_msg, "ERROR, unable to find gamedata object for ID %d", state->selected_gameid);
		ui_StatusMessage(status_msg);
		return UI_OK;
	}
	
	// ===========================
	// Now print out all data, regardless of source
	// ===========================
	tvramPuts(ui_info_name_text_xpos, ui_info_name_text_ypos, ui_progress_font, info_name);
	tvramPuts(ui_info_year_text_xpos, ui_info_year_text_ypos, ui_progress_font, info_year);
	tvramPuts(ui_info_company_text_xpos, ui_info_company_text_ypos, ui_progress_font, info_company);
	tvramPuts(ui_info_genre_text_xpos, ui_info_genre_text_ypos, ui_progress_font, info_genre);
	tvramPuts(ui_info_path_text_xpos, ui_info_path_text_ypos, ui_progress_font, info_path);
	gamedata = gamedata_head;
	return UI_OK;
	
}