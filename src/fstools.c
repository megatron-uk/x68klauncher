/* fstools.c, Filesystem/directory search utilities for x68Launcher.
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
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "newlib_fixes.h"

#ifndef __HAS_DATA
#include "data.h"
#define __HAS_DATA
#endif
#include "fstools.h"

char drvNumToLetter(int drive_number){
	/* Turn a drive number into a drive letter */
	
	static char mapper[MAX_DRIVES] = DRIVE_LETTERS;
	
	if (drive_number > MAX_DRIVES){
		return '\0';	
	} else {
		return mapper[drive_number];
	}
}

int drvLetterToNum(char drive_letter){
	/* Turn a drive letter into a drive number */
	
	int c;
	static char mapper[MAX_DRIVES] = DRIVE_LETTERS;
	
	/* Find the matching position of the drive letter */
	for (c = 0; c < (MAX_DRIVES +1); c++){
		if (mapper[c] == drive_letter){
			return c;	
		}
	}
	return -1;
}

char drvLetterFromPath(char *path){
	/* Return the drive letter part of a path like A:\Games */
	
	/* Must be at least A:\? */
	if (strlen(path) > 3){
		/* Is it a fully qualified path, where the second character is a : */
		if (strncmp(path + 1, ":", 1) == 0){
			/* Return the 'A' part */
			return path[0];
		} else {
			printf("%s.%d\t Doesn't have a drive letter seperator\n", __FILE__, __LINE__);
			return '\0';	
		}
	} else {
		printf("%s.%d\t Seems like a short path\n", __FILE__, __LINE__);
		return '\0';	
	}
}

int dirFromPath(char *path, char *buffer){
	/* Return the directory part of a path like A:\Games\Folder1 */
	
	int sz;
	
	/* Must be at least A:\? */
	if (strlen(path) > 3){
		/* is this a fully qualified path, like a:\games */
		if (strncmp(path + 1, ":", 1) == 0){
			/* size of the string */
			sz = strlen(path);
			if (strncmp(path + sz, "\\", 1) == 0){
				/* copy from the character after the first \ to the last \ */
				strncpy(buffer, path + 3, (sz - 4));
				return 0;
			} else {
				/* copy from the character after the first \ to the last character */
				strncpy(buffer, path + 3, (sz - 3));
				return 0;
			}
		} else {
			return -1;
		}
	} else {
		return -1;	
	}
}

int isDir(char *path){
	/* Boolean test to check if a path is a directory or not */
	char dir;
	int dir_type;
	
	dir = _dos_open(path, 0);
	if (dir == _DOSE_ISDIR){
		dir_type = 1;
	} else {
		dir_type = 0;	
	}
	_dos_close(dir);
	return dir_type;
}

int dirHasData(char *path){
	/* Return 1 if a __launch.dat file is found in a given directory, 0 if missing */
	
	char f;
	int found;
	char filepath[DIR_BUFFER_SIZE];
	
	strcpy(filepath, path);
	strcat(filepath, "\\");
	strcat(filepath, GAMEDAT);
	
	f = _dos_open(filepath, 0);
	if (f > -1){
		found = 1;
	} else {
		found = 0;	
	}
	_dos_close(f);
	return found;
}

int findDirs(char *path, gamedata_t *gamedata, int startnum, config_t *config, launchdat_t *launchdat){
	/* Open a search path and return a count of any directories found, creating a gamedata object for each one. */
	
	// path: Fully qualified path to search, e.g. "A:\Games"
	// gamedata: An instance of the linked-list of game data
	// startnum: The starting number to tag each found 'game' with the next auto-incrementing ID
	
	char drive, old_drive;
	char drive_letter, old_drive_letter;
	char status;
	int go;
	int found;
	
	/* store directory names */
	char old_dir_buffer[DIR_BUFFER_SIZE];
	
	/* hold information about a currently open file */
	struct dos_exfilbuf buffer;
	
	/* hold information about search path */
	char search_drive;
	char search_dirname[DIR_BUFFER_SIZE];
	
	/* initialise counters */
	go = 1;
	found = 0;
	
	/* initialise the directory or search dirname buffer */
	memset(old_dir_buffer, '\0', sizeof(old_dir_buffer));
	memset(search_dirname, '\0', sizeof(search_dirname));
	
	/* split drive and dirname from search path */
	search_drive = drvLetterFromPath(path);
	dirFromPath(path, search_dirname);
	if (FS_VERBOSE){
		printf("%s.%d\t findDirs() Search scope [drive:%c] [path:%s]\n", __FILE__, __LINE__, search_drive, search_dirname);
	}
	
	/* save curdrive */
	old_drive = _dos_curdrv();
	if (old_drive < 0){
		printf("%s.%d\t findDirs() Unable to save current drive [status:%d]\n", __FILE__, __LINE__, old_drive);
		return -1;
	}
	
	/* save curdir */
	status = _dos_curdir((old_drive + 1), old_dir_buffer);
	if (status < 0){
		printf("%s.%d\t findDirs() Unable to save current directory [status:%d]\n", __FILE__, __LINE__, status);
		return -1;
	}
	
	if (isDir(path)){
		/* change to search drive */	
		status = _dos_chgdrv(drvLetterToNum(search_drive));
		if (status < old_drive){
			printf("%s.%d\t findDirs() Unable to change to search drive [status:%d]\n", __FILE__, __LINE__, status);	
		} else {
			
			/* change to search path root */
			status = _dos_chdir("\\");
			if (status != 0){
				printf("%s.%d\t findDirs() Unable to change to search path root [status:%d]\n", __FILE__, __LINE__, status);
			} else {
				
				/* change to actual search path */
				status = (_dos_chdir(search_dirname));
				if (status != 0){
					printf("%s.%d\t findDirs() Unable to change to search path [status:%d][path:%s]\n", __FILE__, __LINE__, status, search_dirname);
				} else {
					
					/* list files with attribute 0x10 == directory and wildcard name. */
					status = _dos_exfiles(&buffer, "*.*", 0x10);	
					if (status >= 0){
						/* Filter out 'special' names */
						if (strcmp(buffer.name, ".") == 0){
							/* we don't want . */
						} else if (strcmp(buffer.name, "..") == 0){
							/* we don't want .. */
						} else {
							if (FS_VERBOSE){
								printf("%s.%d\t findDirs() First Name: %s\n", __FILE__, __LINE__, buffer.name);
							}
							found++;
							//dirHasData()
						}
						/* while there exists some matches from the search.... */
						while (go == 1){
							/* read next match */
							status = _dos_exnfiles(&buffer);
							if (status == 0){
								go = 1;
								if (strcmp(buffer.name, ".") == 0){
									/* we don't want . */
								} else if (strcmp(buffer.name, "..") == 0){
									/* we don't want .. */
								} else {
									memset(search_dirname, '\0', sizeof(search_dirname));
									search_dirname[0] = drvNumToLetter(buffer.driveno);
									strcat(search_dirname, ":");
									strcat(search_dirname, buffer.path);
									strcat(search_dirname, buffer.name);
									
									if (FS_VERBOSE){
										printf("%s.%d\t ID: %d\n", __FILE__, __LINE__, startnum);
										printf("%s.%d\t Name: %s\n", __FILE__, __LINE__, buffer.name);
										printf("%s.%d\t Drive: %c\n", __FILE__, __LINE__, drvNumToLetter(buffer.driveno));
										printf("%s.%d\t Path: %s\n", __FILE__, __LINE__, buffer.path);
										printf("%s.%d\t Full Path: %s\n", __FILE__, __LINE__, search_dirname);
										printf("%s.%d\t Has dat: %d\n", __FILE__, __LINE__, dirHasData(search_dirname));
									}
									found++;
									gamedata = getLastGamedata(gamedata);
									gamedata->next = (gamedata_t *) calloc(sizeof(gamedata_t), 1);
									gamedata->next->gameid = startnum;
									gamedata->next->drive = drvNumToLetter(buffer.driveno);
									strncpy(gamedata->next->path, search_dirname, MAX_PATH_SIZE);
									strncpy(gamedata->next->name, buffer.name, MAX_FILENAME_SIZE);
									gamedata->next->has_dat = dirHasData(search_dirname);
									
									// If pre-loading names from launchdat
									if (gamedata->next->has_dat == 1){
										if (config->preload_names == 1){
											if (FS_VERBOSE){
												printf("%s.%d\t findDirs() Preloading realname\n", __FILE__, __LINE__);
											}
											status = getLaunchdata(gamedata->next, launchdat);
											if (status == 0){
												if (FS_VERBOSE){
													printf("%s.%d\t findDirs() Realname: %s\n", __FILE__, __LINE__, launchdat->realname);
												}
												strncpy(gamedata->next->name, launchdat->realname, MAX_NAME_SIZE);
											} else {
												if (FS_VERBOSE){
													printf("%s.%d\t findDirs() Metadata not found!\n", __FILE__, __LINE__);
												}
											}
										}
									}
									gamedata->next->next = NULL;
									startnum++;
								}
							} else {
								go = 0;
							}
						}
					} else {
						printf("%s.%d\t findDirs() Search for files returned no entries [status:%d]\n", __FILE__, __LINE__, status);
					}
				}
			}
		}
	} else {
		printf("%s.%d\t findDirs() Not a directory\n", __FILE__, __LINE__);
	}
	
	/* reload current drive and directory */
	status = _dos_chgdrv(old_drive);
	if (status < old_drive){
		printf("%s.%d\t findDirs() Unable to restore drive [status:%d]\n", __FILE__, __LINE__, status);
		return -1;	
	}
	status = _dos_chdir("\\");
	if (status != 0){
		printf("%s.%d\t findDirs() Unable to restore root [status:%d]\n", __FILE__, __LINE__, status);
		return -1;
	}
	status = _dos_chdir(old_dir_buffer);
	if (status != 0){
		printf("%s.%d\t findDirs() Unable to restore directory [status:%d\n", __FILE__, __LINE__, status);
		return -1;
	}
	return found;
}

int zeroRunBat(){
	// Empty contents of the run.bat file
	FILE *runbat;
	
	if (FS_VERBOSE){
		printf("%s.%d\t Emptying the %s call file\n", __FILE__, __LINE__, RUNBAT);
	}
	runbat = fopen(RUNBAT, "w");
	if (runbat != NULL){
		fclose(runbat);
	}
	return 0;
}

int writeRunBat(state_t *state, launchdat_t *launchdat){
	// Write a RUN.BAT file in our application directory to launch the state->selected_start exe
	// from the launchdat file at application exit.
	
	FILE *runbat;
	
	runbat = fopen(RUNBAT, "w");
	if (runbat != NULL){
		if (FS_VERBOSE){
			printf("%s.%d\t Opened %s to set game exe\n", __FILE__, __LINE__, RUNBAT);
		}
	} else {
		if (FS_VERBOSE){
			printf("%s.%d\t Unable to write to %s to call game exe\n", __FILE__, __LINE__, RUNBAT);
		}
		return -1;
	}
	
	if (FS_VERBOSE){
		fprintf(runbat, "REM ID: %d\n", state->selected_game->gameid);
		fprintf(runbat, "REM Name: %s\n", state->selected_game->name);
		fprintf(runbat, "REM Drive: %c\n", state->selected_game->drive);
		fprintf(runbat, "REM Path: %s\n", state->selected_game->path);
		fprintf(runbat, "REM Start: %s\n", launchdat->start);
		fprintf(runbat, "REM Alt Start: %s\n", launchdat->alt_start);
		fputs("\n", runbat);
	}
	
	// Change to game drive
	fprintf(runbat, "%c: \n", state->selected_game->drive);

	// CD to game directory
	fprintf(runbat, "cd %s \n", state->selected_game->path);
	
	// Call selected start file
	if (state->selected_start == 0){
		fprintf(runbat, "%s \n", launchdat->start);
	} else {
		fprintf(runbat, "%s \n", launchdat->alt_start);
	}
	
	// Return to original directory
	//fputs("cd -", runbat);
	//fputs("\n", runbat);
	
	// Close run.bat
	fclose(runbat);
	
	return 0;
}