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

#include "data.h"
#include "gfx.h"
#include "textgfx.h"
#include "ui.h"
#include "rgb.h" 
#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

// bmpdata_t structures are needed permanently for all ui 
// bitmap elements, as we may need to repaint the screen at
// periodic intervals after having dialogue boxes or menus open.
bmpdata_t 	*ui_checkbox_bmp;
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

void ui_Close(){
	if (ui_assets_status == UI_ASSETS_LOADED){
		bmp_Destroy(ui_checkbox_bmp);
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
	}
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
	int 	dark, black;
	
	dark = rgb888_2grb(80, 80, 80, 0);
	black = rgb888_2grb(0, 0, 0, 0);
	
	// 1px border around the main box
	
	gvramBox(
		ui_status_bar_xpos, 
		ui_status_bar_ypos, 
		ui_status_bar_xpos + ui_status_bar_width, 
		ui_status_bar_ypos + ui_status_bar_height, 
		dark
	);	
	
	// Inner box, where text goes
	
	gvramBoxFill(
		ui_status_bar_xpos + 1, 
		ui_status_bar_ypos  + 1,
		ui_status_bar_xpos + ui_status_bar_width - 1,
		ui_status_bar_ypos + ui_status_bar_height - 1,
		black
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
	int 	white, lightgrey, blue;
	
	lightgrey = rgb888_2grb(210, 210, 210, 1);
	blue = rgb888_2grb(70, 123, 212, 1);
	
	// Redraw entire graphic
	if (redraw){
		status = gvramBox(
			splash_progress_x_pos, 
			splash_progress_y_pos, 
			splash_progress_x_pos + splash_progress_width, 
			splash_progress_y_pos + splash_progress_height, 
			lightgrey
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
		blue
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
	for (i = 0; i < mid_width; i++){
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
	
	// Checkbox
	ui_asset_reader = fopen(ui_check_box, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_checkbox_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_checkbox_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Checkbox - empty
	ui_asset_reader = fopen(ui_check_box_unchecked, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_checkbox_empty_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_checkbox_empty_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_empty_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Textbox fragments
	ui_asset_reader = fopen(ui_textbox_left, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_textbox_left_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_textbox_left_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_textbox_left_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Textbox fragments
	ui_asset_reader = fopen(ui_textbox_mid, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_textbox_mid_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_textbox_mid_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_textbox_mid_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Textbox fragments
	ui_asset_reader = fopen(ui_textbox_right, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_textbox_right_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_textbox_right_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_textbox_right_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Header
	ui_asset_reader = fopen(ui_header, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_header_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_header_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_header_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Left border
	ui_asset_reader = fopen(ui_border_left, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_border_left_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_border_left_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_border_left_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Middle divider
	ui_asset_reader = fopen(ui_border_central, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_border_divider_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_border_divider_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_border_divider_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Right Border
	ui_asset_reader = fopen(ui_border_right, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_border_right_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_border_right_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_border_right_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);

	// Under artwork
	ui_asset_reader = fopen(ui_under_artwork, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_under_artwork_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_under_artwork_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_under_artwork_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);

	// Under browser
	ui_asset_reader = fopen(ui_under_browser, "rb");
	if (ui_asset_reader == NULL){
		return UI_ERR_FILE;	
	}
	ui_under_browser_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_under_browser_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_under_browser_bmp, 1, 1);
	if (status != 0){
		fclose(ui_asset_reader);
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
	ui_asset_reader = fopen(ui_progress_font_name, "rb");
	if (ui_asset_reader == NULL){
		if (UI_VERBOSE){
			printf("%s.%d\t Error loading UI font data\n", __FILE__, __LINE__);
		}
		return UI_ERR_FILE;	
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
	ui_asset_reader = fopen(ui_status_font_name, "rb");
	if (ui_asset_reader == NULL){
		if (UI_VERBOSE){
			printf("%s.%d\t Error loading UI font data\n", __FILE__, __LINE__);
		}
		return UI_ERR_FILE;	
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
	
	bmp_Destroy(ui_font_bmp);
	return UI_OK;
}

int ui_LoadScreenshot(char *c){
	// Load a game screenshot or cover art into the artwork window coordinates
	int			status;
	bmpdata_t	*screenshot = NULL;
	int			x, y;
	
	ui_StatusMessage("Loading screenshot...");
	
	ui_asset_reader = fopen(c, "rb");
	if (ui_asset_reader == NULL){
		ui_StatusMessage("Error opening screenshot file");
		return UI_ERR_FILE;	
	}
	
	// Read the header to determine size
	ui_StatusMessage("Reading bitmap header...");	
	screenshot = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	screenshot->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, screenshot, 1, 0);
	if (status != 0){
		ui_StatusMessage("Error processing screenshot BMP");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	
	// If image is smaller than artwork window, centre it
	if ((screenshot->width > ui_artwork_width) || (screenshot->height > ui_artwork_height)){
		ui_StatusMessage("Error, image is larger than allowed for artwork");
		bmp_Destroy(screenshot);
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	
	// Read and process bitmap data
	ui_StatusMessage("Reading bitmap data...");	
	status = bmp_ReadImage(ui_asset_reader, screenshot, 0, 1);
	if (status != 0){
		ui_StatusMessage("Error processing screenshot BMP");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	
	ui_StatusMessage("Displaying bitmap...");
	status = gvramBitmap(250, 6, screenshot);
	if (status != 0){
		ui_StatusMessage("Error displaying screenshot BMP");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	
	ui_StatusMessage("Done");
	bmp_Destroy(screenshot);
	fclose(ui_asset_reader);
	return UI_OK;
}

int ui_ProgressMessage(char *c){
	// Output a message in the space above the progress bar

	// Clear the progress message row
	tvramPuts(ui_progress_font_x_pos, ui_progress_font_y_pos, ui_progress_font, "                                             ");
	
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
	
	selected = 1;
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
	char		msg[64];		// Message buffer for each row
	int			startpos;			// Index of first displayable element of state->selected_items
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
	
	// Clear all lines
	y = ui_browser_font_y_pos;
	for(i = 0; i <= ui_browser_max_lines; i++){
		tvramClear8x16(ui_browser_font_x_pos, y, 25);
		y += ui_browser_font_y_pos;
	}
	
	// Display the entries for this page
	gamedata_head = gamedata;
	y = ui_browser_font_y_pos;
	for(i = startpos; i < endpos ; i++){
		gamedata = gamedata_head;
		gameid = state->selected_list[i];
		selected_game = getGameid(gameid, gamedata);
		sprintf(msg, "%s", selected_game->name);
		tvramPuts(ui_browser_font_x_pos, y, ui_progress_font, msg);
		y += ui_browser_font_y_pos;
	}
	gamedata = gamedata_head;
	
	return UI_OK;
}

int ui_UpdateBrowserPaneStatus(state_t *state){
	// Draw browser pane status message in status panel
	char	msg[64];		// Message buffer for the status bar
	
	sprintf(msg, "Line %02d/%02d      Page %02d/%02d", state->selected_line, ui_browser_max_lines, state->selected_page, state->total_pages);
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
	
	gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	//launchdat_t	*launchdat;		// Metadata object representing the launch.dat file for this game
	
	// Store gamedata head
	gamedata_head = gamedata;
	
	// Find the game data entry for this ID
	//selected_game = getGameid(state->selected_gameid, gamedata);
	
	// Clear all existing text
	tvramClear8x16(ui_info_name_text_xpos, ui_info_name_text_ypos, 32);
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
					sprintf(info_name, " %s", launchdat->realname);
				} else {
					sprintf(info_name, " %s", state->selected_game->name);
				}
				
				if (launchdat->genre != NULL){
					sprintf(info_genre, "%s", launchdat->genre);
				} else {
					sprintf(info_genre, "N/A");
				}
				
				if (launchdat->year != 0){
					sprintf(info_year, "%d", launchdat->year);
				} else {
					sprintf(info_year, "N/A");
				}
				
				if ((strlen(launchdat->developer) > 0) && (strlen(launchdat->publisher) > 0)){
					sprintf(info_company, " %s / %s", launchdat->developer, launchdat->publisher);
				} else if (strlen(launchdat->developer) > 0){
					sprintf(info_company, " %s", launchdat->developer);
				} else if (strlen(launchdat->publisher) > 0){
					sprintf(info_company, " %s", launchdat->publisher);
				} else {
					sprintf(info_company, " N/A");
				}
				
				// Start file
				
				// Number of images/screenshots
	
				sprintf(info_path, " %s", state->selected_game->path);
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