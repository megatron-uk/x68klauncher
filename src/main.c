/* main.c, Main executable segment for the x68Launcher.
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

#include <dos.h>
#include <iocs.h>
#include <ini.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "fstools.h"
#include "gfx.h"
#include "input.h"
#include "rgb.h"
#include "textgfx.h"
#include "ui.h"
#ifndef __HAS_MAIN
#include "main.h"
#define __HAS_MAIN
#endif

int main() {
	/* Lets get this show on the road!!! */
	
	uint16_t black;
	int has_screenshot;
	int has_launchdat;
	int has_images;
	int old_gameid;
	int i;								// Loop counter
	int active_pane;						// Indicator of which UI element is active and consuming input
	int exit;								// Status flag indicating user wants to quit
	int user_input, joy_input, key_input;		// User input state - either a keyboard code or joystick direction/button
	int scrape_dirs;						// NUmber of directories being scraped
	int scrape_progress_chunk_size;		// Size of progress bar increase per directory being scraped
	int progress;							// Progress bar percentage
	int super;							// Supervisor mode flag
	int found, found_tmp;					// Number of gamedirs/games found
	int verbose;							// Controls output of additional logging/text
	int status;							// Generic function return status variable
	char msg[64];						// Message buffer
	FILE *screenshot_file;
	int savefile;
	
	black = rgb888_2grb(0,0,0,0);
	
	state_t *state = NULL;
	bmpdata_t *screenshot_bmp = NULL;	
	bmpstate_t *screenshot_bmp_state = NULL;	
	bmpdata_t *game_bmp = NULL;			// Loads a cover/screenshot for a game 
	gamedata_t *gamedata = NULL;		// An initial gamedata record for the first game directory we read
	gamedata_t *gamedata_head = NULL;	// Constant pointer to the start of the gamedata list
	launchdat_t *launchdat = NULL;			// When a single game is selected, we attempt to load its metadata file from disk
	imagefile_t *imagefile = NULL;			// When a single game is selected, we attempt to load a list of the screenshots from metadata
	imagefile_t *imagefile_head = NULL;		// Constant pointer to the start of the game screenshot list
	gamedir_t *gamedir = NULL;			// List of the game search directories, as defined in our INIFILE
	config_t *config = NULL;				// Configuration data as defined in our INIFILE
		
	screenshot_file = NULL;
	has_screenshot = 0;
	has_launchdat = 0;
	has_images = 0;
	old_gameid = -1;
	active_pane = BROWSER_PANE;			// Set initial focus to browser pane
	user_input = joy_input = key_input = 0;	// Initial state of all input variables
	exit = 0;								// Dont exit from main loop unless specified
	scrape_dirs = 0;						// Default to 0 directories found
	progress = 0;							// Default to 0 progress bar size
	found = found_tmp = 0;				// Counter of the number of found directories/gamedata items
	verbose = 1;							// Initial debug/verbose setting; overidden from INIFILE, if set
	super = _dos_super(0);				// Enter supervisor mode, so we can access graphics memory
	
	printf("%s starting...\n", MY_NAME);
	
	/* ************************************** */
	/* Create a new empty gamedata entry */
	/* ************************************** */
	gamedata = (gamedata_t *) malloc(sizeof(gamedata_t));
	gamedata->next = NULL;
	
	/* ************************************** */
	/* Parse the gamedirs that are set */
	/* ************************************** */
	gamedir = (gamedir_t *) malloc(sizeof(gamedir_t));
	gamedir->next = NULL;
	
	/* ************************************** */
	/* Create an instance of a config data */
	/* ************************************** */
	config = (config_t *) malloc(sizeof(config_t));
	config->dir = NULL;
	
	screenshot_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	screenshot_bmp->pixels = NULL;
	
	screenshot_bmp_state = (bmpstate_t *) malloc(sizeof(bmpstate_t));
	screenshot_bmp_state->pixels = NULL;
	
	/* ************************************** */
	/* Create an instance of the UI state data */
	/* ************************************** */
	state = (state_t *) malloc(sizeof(state_t));
	state->selected_max = 0;			// Total amount of items in current filtered selection
	state->selected_page = 1;			// Default to first page of selected games 
	state->selected_line = 0;			// Default to first line selected
	state->total_pages = 0;			// Total number of pages of selected games (selected_max / ui_browser_max_lines)
	state->selected_gameid = -1;		// Current selected game
	state->has_images = 0;
	state->has_launchdat = 0;
	state->active_pane = BROWSER_PANE;
	for(i =0; i <selection_list_size; i++){
		state->selected_list[i] = NULL;
	}
	
	/* ************************************** */
	/* Parse our ini file */
	/* ************************************** */
	if (verbose){
		printf("%s.%d\t Loading configuration\n", __FILE__, __LINE__);
	}
	status = getIni(config, verbose);
	if (status != 0){
		printf("%s.%d\t Error unable to parse config file!!!\n", __FILE__, __LINE__);
		printf("\n");
		printf("\n");
		printf("You must have a config file named %s in the same directory as this file\n", INIFILE);
		printf("It should have, at a minimum, the following entries:\n");
		printf("\n");
		printf("[defaults]\n");
		printf("gamedirs=ABC123\n");
		printf("\n");
		printf("Where ABC123 should be replaced by a comma seperate list of one or more paths to your games (e.g. A:\\Games)\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return 1;
	} else {
		printf("\n");
		printf("%s Configuration\n", MY_NAME);
		printf("=========================\n");
		printf("[default]\n");
		printf("verbose=%d\n", config->verbose);
		printf("gamedirs=%s\n", config->dirs);
		printf("\n");
		if (config->verbose == 0){
			printf("Verbose mode is disabled, you will not receive any further logging after this point\n");
			printf("If required, enable verbose mode by adding: verbose=1 to %s\n", INIFILE);
			printf("\n");
		}
	}
		
	// ======================
	// Initialise GUI 
	// ======================
	status = gfx_Init(1, 1);
	if (status != 0){
		printf("ERROR! Unable to initialise graphics mode!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;	
	}
	txt_Init();
	txt_Clear();
	
	status = ui_DrawSplash();
	if (status != 0){
		printf("ERROR! Unable to load asset data for initial splash screen!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;	
	}
	ui_DrawSplashProgress(1, progress);
	
	// ======================
	// Load UI font data
	// ======================
	status = ui_LoadFonts();
	if (status != UI_OK){
		printf("ERROR! Unable to load asset data for user interface!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}
	ui_ProgressMessage("Font loaded!");
	progress += splash_progress_chunk_size;
	ui_DrawSplashProgress(0, progress);
	
	// ======================
	// Load UI asset data
	// ======================
	ui_ProgressMessage("Loading UI assets...");
	status = ui_LoadAssets();
	if (status != UI_OK){
		printf("ERROR! Unable to load asset data for user interface!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}
	progress += splash_progress_chunk_size;
	ui_DrawSplashProgress(0, progress);
	
	// ======================
	// Apply any settings from the config file
	// ... including extracting the search directory
	// names.
	// ======================
	ui_ProgressMessage("Parsing configuration...");
	if (strlen(config->dirs) > 3){
		status = getDirList(config, gamedir, config->verbose);
		if (status < 1){
			printf("%s.%d\t Error when extracting game search directories!!!\n", __FILE__, __LINE__);
			printf("\n");
			printf("\n");
			printf("None of the directories listed in %s under the gamedirs setting could be found.\n", INIFILE);
			printf("Check your gamedirs setting and ensure you have at least one directory that exists.\n");
			printf("e.g.\n");
			printf("\n");
			printf("[defaults]\n");
			printf("gamedirs=A:\\Games,A:\\MoreGames,C:\\Stuff\\EvenMoreGames\n");
			printf("\n");
			free(config);
			free(gamedir);
			free(gamedata);
			ui_Close();
			gfx_Close();
			return -1;
		}
	} else {
		printf("%s.%d\t Error when attempting to load game search directories!!!\n", __FILE__, __LINE__);
		printf("\n");
		printf("\n");
		printf("You must have at least one path listed in %s for the gamedirs setting.\n", INIFILE);
		printf("Remember you can use backslashes OR the Yen symbol in path names.\n");
		printf("e.g.\n");
		printf("\n");
		printf("[defaults]\n");
		printf("gamedirs=A:\\Games,A:\\MoreGames,C:\\Stuff\\EvenMoreGames\n");
		printf("\n");
		
		free(config);
		free(gamedir);
		free(gamedata);
		ui_Close();
		gfx_Close();
		return -1;
	}
	progress += splash_progress_chunk_size;
	ui_DrawSplashProgress(0, progress);
	ui_ProgressMessage("Configuration applied!");
	
	// ======================
	//
	// This section here loops through our game search paths and finds
	// and subdirectories (that should contain games). 
	//
	// ======================
	ui_ProgressMessage("Adding search paths...");
	gamedir = config->dir;
	while (gamedir->next != NULL){
		gamedir = gamedir->next;
		scrape_dirs++;
	}
	progress += splash_progress_chunk_size;
	ui_DrawSplashProgress(0, progress);
	
	// Calculate progress size for each dir scraped
	scrape_progress_chunk_size = splash_progress_chunk_size / scrape_dirs;
	sprintf(msg, "Scraping %d directories for content...", scrape_dirs);
	ui_ProgressMessage(msg);
	gamedir = config->dir;
	while (gamedir->next != NULL){
		gamedir = gamedir->next;
		// ======================
		//
		// Show graphical progress update for this directory scraping
		//
		// ======================		
		found_tmp = 0;
		found_tmp = findDirs(gamedir->path, gamedata, found);
		found = found + found_tmp;
		sprintf(msg, "Found %d games in %s", found_tmp, gamedir->path);
		ui_ProgressMessage(msg);
		progress += scrape_progress_chunk_size;
		ui_DrawSplashProgress(0, progress);
	}	
	ui_ProgressMessage("Scraped!");
	
	// ========================
	//
	// No gamedirs were found
	//
	// ========================
	if (found < 1){
		free(config);
		free(gamedir);
		free(gamedata);
		ui_Close();
		gfx_Close();
		printf("%s.%d\t Error no game folders found while scraping your directories!!!\n", __FILE__, __LINE__);
		printf("\n");
		printf("\n");
		printf("You must have at least one path listed in %s for the gamedirs setting.\n", INIFILE);
		printf("Remember you can use backslashes OR the Yen symbol in path names.\n");
		printf("e.g.\n");
		printf("\n");
		printf("[defaults]\n");
		printf("gamedirs=A:\\Games,A:\\MoreGames,C:\\Stuff\\EvenMoreGames\n");
		printf("\n");
		return -1;
	}
	
	// =========================
	//
	// Sort the game entries by name
	//
	// =========================
	sprintf(msg, "Sorting %d games...", found);
	ui_ProgressMessage(msg);
	sortGamedata(gamedata, config->verbose);
	progress += splash_progress_chunk_size;
	ui_DrawSplashProgress(0, progress);
	ui_ProgressMessage("Sorted!");
	
	// ======================
	// 
	// Do an initial selection list of all titles
	//
	// ======================
	ui_ProgressMessage("Building initial selection list...");
	if (gamedata->next != NULL){
		gamedata = gamedata->next;
	}
	i= 0;
	gamedata_head = gamedata; // Store first item
	while(gamedata->next != NULL){
		state->selected_list[i] = gamedata->gameid;
		gamedata = gamedata->next;
		i++;
	}
	gamedata = gamedata_head; // Restore first item
	state->selected_max = i;
	state->selected_page = 1;
	state->selected_line = 1;
	state->total_pages = 0;
	state->selected_gameid = 0;
	for(i = 0; i < state->selected_max ; i++){
		if (i % ui_browser_max_lines == 0){
			state->total_pages++;
		}
	}
	
	// ======================
	//
	// Write a gamelist text file
	//
	// ======================
	if (config->save){
		sprintf(msg, "Saving game list to %s", SAVEFILE);
		ui_ProgressMessage(msg);
		
		_dos_delete(SAVEFILE);
		savefile = _dos_create(SAVEFILE, 0x8000);
		if (savefile < 0){
			sprintf(msg, "Warning: Unable to create save file. Press any key.");
			ui_ProgressMessage(msg);
			_dos_getc();
		} else {
			gamedata_head = gamedata;
			while(gamedata->next != NULL){
				_dos_fputs(gamedata->path, savefile);
				_dos_fputs("\n", savefile);
				gamedata = gamedata->next;
			}
			gamedata = gamedata_head;
			_dos_close(savefile);
		}
	} else {
		ui_ProgressMessage("Not saving game list...");
	}
	ui_DrawSplashProgress(0, splash_progress_complete);
	
	// ======================
	// Load user interface and populate
	// initial window contents
	// ======================
	ui_ProgressMessage("Waiting to load main UI...");
	txt_Clear();
	status = ui_DrawMainWindow();
	if (status != UI_OK){
		printf("ERROR! Unable to load draw main UI window!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}
	status = ui_DrawStatusBar();
	if (status != UI_OK){
		printf("ERROR! Unable to draw status bar!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}
	status = ui_DrawInfoBox();
	if (status != UI_OK){
		printf("ERROR! Unable to draw info pane widgets!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}
	status = ui_UpdateBrowserPane(state, gamedata);
	if (status != UI_OK){
		printf("ERROR! Unable to update browser pane contents!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}	
	status = ui_UpdateInfoPane(state, gamedata, launchdat);
	if (status != UI_OK){
		printf("ERROR! Unable to update info pane contents!\n");
		free(config);
		free(gamedir);
		free(gamedata);
		return status;
	}
	
	// ======================
	//
	// Main loop here
	//
	// ======================	
	ui_StatusMessage("Waiting for user input...");
	while (exit == 0){
		
		// Next frame of animation for game selector
		// TO DO
		
		// Check for keypress / joystick input
		user_input = input_get();
		
		// ==================================
		//
		// We are in the game browser window
		//
		// ==================================
		if (active_pane == BROWSER_PANE){
			switch(user_input){
				case(input_quit):
					// Exit the application
					exit = 1;
					break;
				case(input_up):
					// Move game selector up by one line
					if (state->selected_line == 1){
						if (state->selected_page == 1){
							// Loop back to last page
							state->selected_page = state->total_pages;
							ui_UpdateBrowserPane(state, gamedata);
						} else {
							// Go back one page
							state->selected_page--;
							ui_UpdateBrowserPane(state, gamedata);
						}
						// Reset to line 1 of the new page
						state->selected_line = 1;							
					} else {
						// Move up one line
						state->selected_line--;
					}
					break;
				case(input_down):
					// Move game selector down by one line
					if (state->selected_line == ui_browser_max_lines){
						if (state->selected_page == state->total_pages){
							// Go to first page
							state->selected_page = 1;
							ui_UpdateBrowserPane(state, gamedata);
						} else {
							// Go forward one page
							state->selected_page++;
							ui_UpdateBrowserPane(state, gamedata);
						}
						// Reset to line 1 of the new page
						state->selected_line = 1;							
					} else {
						// Move down one line
						state->selected_line++;
					}
					break;
				case(input_scroll_up):
					// Scroll up by an entire page of lines
					state->selected_line = 1;
					if (state->selected_page == 1){
						// Loop back to last page
						state->selected_page = state->total_pages;
						ui_UpdateBrowserPane(state, gamedata);
					} else {
						// Back one page
						state->selected_page--;
						ui_UpdateBrowserPane(state, gamedata);
					}
					break;					
				case(input_scroll_down):
					// Scroll down by an entire page of lines
					state->selected_line = 1;
					if (state->selected_page == state->total_pages){
						// Go back to first page
						state->selected_page = 1;
						ui_UpdateBrowserPane(state, gamedata);
					} else {
						// Forward one page
						state->selected_page++;
						ui_UpdateBrowserPane(state, gamedata);
					}
					break;
				case(input_left):
					// Scroll left through artwork - if available
					// ...
					// Loop back to last image
					break;
				case(input_right):
					// Scroll right through artwork - if available
					if (imagefile != NULL){
						if (imagefile->next != NULL){
							imagefile = imagefile->next;
							
						} else {
							imagefile = imagefile_head;
						}
						// Restart artwork display
						// =======================
						// Close previous screenshot file handle
						// =======================
						if (screenshot_file){
							fclose(screenshot_file);
							screenshot_file = NULL;
						}
						
						sprintf(msg, "%s\\%s", state->selected_game->path, imagefile->next->filename);
						strcpy(state->selected_image, msg);
						
						// =======================
						// Open new screenshot file, ready parse
						// =======================
						screenshot_file = fopen(state->selected_image, "rb");
						if (screenshot_file == NULL){
							has_screenshot = 0;
						} 
						else {
							// =======================
							// Load header of screenshot bmp
							// =======================
							status = bmp_ReadImage(screenshot_file, screenshot_bmp, 1, 0);
							if (status != 0){
								ui_StatusMessage("Error, unable to read image header!");
								has_screenshot = 0;
							} else {
								has_screenshot = 1;	
								sprintf(msg, "Header read: %d x %d @ %d bpp", screenshot_bmp->width, screenshot_bmp->height, screenshot_bmp->bpp);
								ui_StatusMessage(msg);
								screenshot_bmp_state->rows_remaining = screenshot_bmp->height;
								gvramBoxFill(ui_artwork_xpos, ui_artwork_ypos, ui_artwork_xpos + 256, ui_artwork_ypos + 256, black);
							}
						}
					}
					break;
				/*
				case(input_switch):
					ui_SwitchPane(state);
					break;
				*/
			}
			// Detect if selected game has changed
			ui_ReselectCurrentGame(state);
			
			// Only refresh browser, artwork and info panes if the selected game has changed
			if (old_gameid != state->selected_gameid){
				// ======================
				// Destroy current list of artwork
				// ======================
				if (has_images){
					imagefile = imagefile_head;
					removeImagefile(imagefile);
					if (imagefile != NULL){
						free(imagefile);
					}
				}
				// Clear artwork window
				gvramBoxFill(ui_artwork_xpos, ui_artwork_ypos, ui_artwork_xpos + 256, ui_artwork_ypos + 256, black);
				memset(state->selected_image, '\0', sizeof(state->selected_image)); 
				state->has_images = 0;
				
				// ======================
				// Destroy previous launch.dat
				// ======================
				if (has_launchdat){
					if (launchdat != NULL){
						free(launchdat);
					}
				}
				state->has_launchdat = 0;
				
				// ======================
				// Update selection to current game
				// ======================
				gamedata_head = gamedata;
				state->selected_game = getGameid(state->selected_gameid, gamedata);
				if (state->selected_game->has_dat){
					launchdat = (launchdat_t *) malloc(sizeof(launchdat_t));	
					status = getLaunchdata(state->selected_game, launchdat);
					if (status != 0){
						ui_StatusMessage("Error, could not load metadata!");
					} else {
						state->has_launchdat = 1;
					}
				}
				
				// ======================
				// Load list of artwork
				// ======================
				if (state->has_launchdat){
					imagefile = (imagefile_t *) malloc(sizeof(imagefile_t));
					imagefile->next = NULL;
					imagefile_head = imagefile;
					status = getImageList(launchdat, imagefile);
					if (status > 0){
						state->has_images = 1;
					}
				}
								
				// =======================
				// Select first screenshot/artwork to show
				// =======================
				if (state->has_images){
					if (imagefile->next != NULL){
						sprintf(msg, "%s\\%s", state->selected_game->path, imagefile->next->filename);
						strcpy(state->selected_image, msg);
					}
				}
				
				// =======================
				// Updating info and browser pane
				// =======================
				gamedata = gamedata_head;
				ui_UpdateInfoPane(state, gamedata, launchdat);
				ui_UpdateBrowserPaneStatus(state);
				old_gameid = state->selected_gameid;

				// =======================
				// Close previous screenshot file handle
				// =======================
				if (screenshot_file){
					fclose(screenshot_file);
					screenshot_file = NULL;
				}
				
				// =======================
				// Open new screenshot file, ready parse
				// =======================
				screenshot_file = fopen(state->selected_image, "rb");
				if (screenshot_file == NULL){
					has_screenshot = 0;
				} 
				else {
					// =======================
					// Load header of screenshot bmp
					// =======================
					status = bmp_ReadImage(screenshot_file, screenshot_bmp, 1, 0);
					if (status != 0){
						ui_StatusMessage("Error, unable to read image header!");
						has_screenshot = 0;
					} else {
						has_screenshot = 1;	
						sprintf(msg, "Header read: %d x %d @ %d bpp", screenshot_bmp->width, screenshot_bmp->height, screenshot_bmp->bpp);
						ui_StatusMessage(msg);
						screenshot_bmp_state->rows_remaining = screenshot_bmp->height;
					}
				}
			}
			
		// ==================================
		//
		// We are in the artwork/screenshot window
		//
		// ==================================
		/*
		} else if (active_pane == ARTWORK_PANE){
			switch(user_input){
				case(input_quit):
					exit = 1;
					break;
				case(input_switch):
					ui_SwitchPane(state);
					break;
			}
		*/	
		// ==================================
		//
		// We are in the bottom info window
		//
		// ==================================
		/*
		} else if (active_pane == INFO_PANE){
			switch(user_input){
				case(input_quit):
					exit = 1;
					break;
				case(input_switch):
					ui_SwitchPane(state);
					break;
			}
		*/
		}

		// ========================================
		//
		// After all user input is handled...
		// Load screenshot for current selected game asynchronously 
		// Ok, well, not really, just one line at a time so that it looks like it
		// is happening in the background.
		//		
		// Only do this if the current game has a screenshot(s)
		//
		// ========================================
		if ((has_screenshot) && (screenshot_bmp_state->rows_remaining > 0)){
			status = gvramBitmapAsync(ui_artwork_xpos, ui_artwork_ypos, screenshot_bmp, screenshot_file, screenshot_bmp_state);
			switch(status){
				case(GFX_ERR_UNSUPPORTED_BPP):
					ui_StatusMessage("Artwork is an unsupported colour depth.");
					has_screenshot = 0;
					break;
				case(GFX_ERR_MISSING_BMPHEADER):
					ui_StatusMessage("No image header supplied to async display.");
					has_screenshot = 0;
					break;
				case(BMP_ERR_READ):
					ui_StatusMessage("Error seeking within image file.");
					has_screenshot = 0;
					break;
				case(0):
					break;
				default:
					ui_StatusMessage("Unhandled return code from async display.");
					has_screenshot = 0;
					break;
			}
		}
		
	}
	/*
	while(gamedata->next != NULL){
		if (gamedata->has_dat == 1){
			
			// Load launch.dat file
			launchdat = (launchdat_t *) malloc(sizeof(launchdat_t));
			getLaunchdata(gamedata, launchdat, verbose);
			printf("%s.%d\t Realname: %s\n", __FILE__, __LINE__, launchdat->realname);
			
			// Parse any image filenames
			imagefile = (imagefile_t *) malloc(sizeof(imagefile_t));
			imagefile->next = NULL;
			getImageList(launchdat, imagefile, verbose);
			imagefile_head = imagefile;
			while (imagefile->next != NULL){
				imagefile = imagefile->next;
				printf("%s.%d\t Image: %s\n", __FILE__, __LINE__, imagefile->filename);
			}		
			// Free memory used by launch.dat
			free(launchdat);
			
			// Free memory used by image list
			removeImagefile(imagefile_head, verbose);
		}
		gamedata = gamedata->next;
	}
	// Free memory used by gamedata list
	removeGamedata(gamedata_head, verbose);
	*/
	
	free(config);
	free(gamedir);
	free(gamedata);
	txt_Clear();
	txt_Close();
	gfx_Close();
	return 0;
}