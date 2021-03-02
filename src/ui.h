/* ui.h, User interface structure and bmp filenames for the x68Launcher.
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

#include "rgb.h" 
#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif
#ifndef __HAS_MAIN
#include "main.h"
#define __HAS_MAIN
#endif

// Enable/disable logging for the ui.c functions
#define UI_VERBOSE					0

#define splash_logo					"assets\\logo.bmp"
#define splash_progress_x_pos		56
#define splash_progress_y_pos		490
#define splash_progress_width		404
#define splash_progress_height		14
#define splash_progress_chunks		7
#define splash_progress_chunk_size	((splash_progress_width - 4) / splash_progress_chunks)
#define splash_progress_complete 	(splash_progress_width - 4)

// Progress Font filename
#define ui_progress_font_name		"assets\\font8x16.bmp"
#define ui_progress_font_width		8   // Font symbol width
#define ui_progress_font_height		16 // Font symbol height
#define ui_progress_font_ascii_start	33 // Font table starts at ascii '!'
#define ui_progress_font_total_syms	96 // Total number of symbols in font table
#define ui_progress_font_unknown		95 // Use ascii '?' for missing symbols
#define ui_progress_font_x_pos		4
#define ui_progress_font_y_pos		(splash_progress_y_pos - (ui_progress_font_height + 2))

// Dark metal theme
#define ui_header				"assets\\dark\\border_header.bmp"
#define ui_border_left			"assets\\dark\\border_left.bmp"
#define ui_border_right			"assets\\dark\\border_right.bmp"
#define ui_border_central		"assets\\dark\\border_divider.bmp"
#define ui_under_artwork			"assets\\dark\\below_art.bmp"
#define ui_under_browser			"assets\\dark\\below_browser.bmp"
#define ui_check_box				"assets\\dark\\check_box.bmp"
#define ui_check_box_choose		"assets\\dark\\check_box_choos.bmp"
#define ui_check_box_unchecked	"assets\\dark\\check_box_empty.bmp"
#define ui_textbox_left			"assets\\dark\\textbox_8x16_left.bmp"
#define ui_textbox_mid			"assets\\dark\\textbox_8x16_midb.bmp"
#define ui_textbox_right			"assets\\dark\\textbox_8x16_right.bmp"
#define ui_select				"assets\\dark\\select.bmp"

// Coordinates
#define ui_header_xpos				0
#define ui_header_ypos				0
#define ui_border_left_xpos			0
#define ui_border_left_ypos			12
#define ui_border_central_xpos		230
#define ui_border_central_ypos		12
#define ui_border_right_xpos			(GFX_COLS - 12)
#define ui_border_right_ypos			12
#define ui_under_artwork_xpos		230
#define ui_under_artwork_ypos		268
#define ui_under_browser_xpos		0
#define ui_under_browser_ypos		386

// Location of checkbox widgets 
#define ui_checkbox_has_metadata_xpos				242
#define ui_checkbox_has_metadata_ypos				280
#define ui_checkbox_has_startbat_xpos				242
#define ui_checkbox_has_startbat_ypos				(ui_checkbox_has_metadata_ypos + 22 + 6)
#define ui_checkbox_has_metadata_startbat_xpos		242
#define ui_checkbox_has_metadata_startbat_ypos		(ui_checkbox_has_startbat_ypos + 22 + 6)
#define ui_checkbox_has_images_xpos					242
#define ui_checkbox_has_images_ypos					(ui_checkbox_has_metadata_startbat_ypos + 22 + 6)

// Artwork window
#define ui_artwork_xpos				244
#define ui_artwork_ypos				12

// Location of info pane textbox widgets
#define ui_info_name_xpos			99
#define ui_info_name_ypos			398
#define ui_info_name_width			274
#define ui_info_name_text_xpos		(ui_info_name_xpos / 16)
#define ui_info_name_text_ypos		402

#define ui_info_year_xpos			438
#define ui_info_year_ypos			398
#define ui_info_year_width			64
#define ui_info_year_text_xpos		(ui_info_year_xpos / 16) + 1
#define ui_info_year_text_ypos		402

#define ui_info_company_xpos			99
#define ui_info_company_ypos			430
#define ui_info_company_width		215
#define ui_info_company_text_xpos	(ui_info_company_xpos / 16)
#define ui_info_company_text_ypos	434

#define ui_info_genre_xpos			390
#define ui_info_genre_ypos			430
#define ui_info_genre_width			112
#define ui_info_genre_text_xpos		(ui_info_genre_xpos / 16) + 1
#define ui_info_genre_text_ypos		434

#define ui_info_path_xpos			99
#define ui_info_path_ypos			462
#define ui_info_path_width			403
#define ui_info_path_text_xpos		(ui_info_path_xpos / 16)
#define ui_info_path_text_ypos		466

// launch window popup
#define ui_launch_popup_xpos			100
#define ui_launch_popup_ypos			150
#define ui_launch_popup_width		300
#define ui_launch_popup_height		100

// artwork window dimensions
#define ui_artwork_width				256	// Width of the window that holds artwork
#define ui_artwork_height			256 // Height of the window that holds artwork

// status bar dimensions
#define ui_status_font_name			"assets\\font8x8.bmp"
#define ui_status_font_width			8   // Font symbol width
#define ui_status_font_height		8 // Font symbol height
#define ui_status_font_ascii_start	32 // Font table starts at ascii ' '
#define ui_status_font_total_syms	96 // Total number of symbols in font table
#define ui_status_font_unknown		31 // Use ascii '?' for missing symbols
#define ui_status_bar_xpos			10
#define ui_status_bar_ypos			496
#define ui_status_bar_width			400
#define ui_status_bar_height			12   // 8px for font, plus 2px above and below
#define ui_status_font_x_pos			1
#define ui_status_font_y_pos			(ui_status_bar_ypos + 2)

// Info panel
#define ui_info_panel_texture		"assets\\info_texture.bmp"
#define ui_info_panel_texture_x_pos 	ui_border_width
#define ui_info_panel_texture_y_pos (384 + ui_header_width)
#define ui_info_panel_x_min			ui_border_width
#define ui_info_panel_x_max			(GFX_COLS - ui_border_width)
#define ui_info_panel_y_pos			384

// Browser/artwork divider
#define ui_browser_panel_x_pos		((GFX_COLS / 2) - (ui_border_width * 2))
#define ui_browser_panel_y_min		ui_header_height
#define ui_browser_panel_y_max		384
#define ui_browser_font_x_pos		1
#define ui_browser_font_y_pos		15
#define ui_browser_max_lines			19
#define ui_browser_footer_font_xpos	1
#define ui_browser_footer_font_ypos	374
#define ui_browser_cursor_xpos 		13

// Return codes
#define UI_OK					0
#define UI_ERR_FILE				-1
#define UI_ERR_BMP				-2
#define UI_ERR_FUNCTION_CALL		-3
#define UI_ASSETS_LOADED			255
#define UI_ASSETS_MISSING		254

// Labels for active panes
#define PANE_FIRST				0x01
#define BROWSER_PANE				0x01
#define ARTWORK_PANE				0x02
#define INFO_PANE				0x03
#define INFO_PANE				0x03
#define LAUNCH_PANE				0x04
#define CONFIRM_PANE				0x05
#define FILTER_PRE_PANE			0x06
#define FILTER_PANE				0x07
#define HELP_PANE				0x08
#define PANE_MAX					0x08

// Colours
uint16_t PALETTE_UI_BLACK;
uint16_t PALETTE_UI_WHITE;
uint16_t PALETTE_UI_LGREY;
uint16_t PALETTE_UI_MGREY;
uint16_t PALETTE_UI_DGREY;
uint16_t PALETTE_UI_RED;
uint16_t PALETTE_UI_GREEN;
uint16_t PALETTE_UI_BLUE;
uint16_t PALETTE_UI_YELLOW;

#define PALETTE_UI_WHITE_TEXT 1
#define PALETTE_UI_LGREY_TEXT 2
#define PALETTE_UI_MGREY_TEXT 3
#define PALETTE_UI_DGREY_TEXT 4
#define PALETTE_UI_RED_TEXT 5
#define PALETTE_UI_GREEN_TEXT 6
#define PALETTE_UI_BLUE_TEXT 7
#define PALETTE_UI_YELLOW_TEXT 8

// Functions
void		ui_Init();
void		ui_Close();

// These draw the basic UI elements
int		ui_DrawConfirmPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat);
int 		ui_DrawFilterPrePopup(state_t *state, int toggle);
int 		ui_DrawFilterPopup(state_t *state, int select, int redraw, int toggle);
int		ui_DrawHelpPopup();
int		ui_DrawInfoBox();
int		ui_DrawLaunchPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat, int toggle);
int		ui_DrawMainWindow();
int		ui_DrawMultiChoiceFilterPopup(state_t *state, int select, int redraw, int toggle);
int		ui_DrawSplash();
int		ui_DrawSplashProgress();
int		ui_DrawStatusBar();
int		ui_DrawTextPanel(int x, int y, int width);

// Asset loaders
int		ui_LoadAssets();
int		ui_LoadFonts();
int		ui_LoadScreenshot(char *c);
int 		ui_DisplayArtwork(FILE *screenshot_file, bmpdata_t *screenshot_bmp, bmpstate_t *screenshot_state, state_t *state, imagefile_t *imagefile);

// Output error or status messages
int		ui_ProgressMessage(char *c);
int		ui_StatusMessage(char *c);

// Change focus or selected state of UI elements
int		ui_SwitchPane(state_t *state);
int		ui_ReselectCurrentGame(state_t *state);

// These refresh contents within the various UI elements
int		ui_UpdateBrowserPane(state_t *state, gamedata_t *gamedata);
int		ui_UpdateBrowserPaneStatus(state_t *state);
int		ui_UpdateInfoPane(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat);
