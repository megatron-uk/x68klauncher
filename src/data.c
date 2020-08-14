/* data.c, Tools to add or manipulate game data for x68Launcher.
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

#include "ini.h"
#include "data.h"
#include "fstools.h"
#include "main.h"

gamedata_t * getGameid(int gameid, gamedata_t *gamedata){
	// Find a given gameid from the list	
	
	while (gamedata->next != NULL){
		if (gamedata->gameid == gameid){
			return gamedata;
		}
		gamedata = gamedata->next;
	}
	return NULL;
}

gamedata_t * getLastGamedata(gamedata_t *gamedata){
	/* Given a gamedata item, find the last entry of the list */
	
	while (gamedata->next != NULL){
		gamedata = gamedata->next;
	}
	return gamedata;
}


imagefile_t * getLastImage(imagefile_t *imagefile){
	/* Given a imagefile item, find the last entry of the list */
	
	while (imagefile->next != NULL){
		imagefile = imagefile->next;
	}
	return imagefile;	
}

gamedir_t * getLastGameDir(gamedir_t *gamedir){
	/* Given a gamedir search path item, find the last entry of the list */
	
	while (gamedir->next != NULL){
		gamedir = gamedir->next;
	}
	return gamedir;
}

int removeGamedata(gamedata_t *gamedata, int verbose){
	/* Remove all nodes of a given imagefile list */
	
	gamedata_t * current = gamedata;
	gamedata_t * next = NULL;
	
	if (current->next == NULL){
		if (verbose){
			printf("%s.%d\t Freeing gamedata single object\n", __FILE__, __LINE__);	
		}
		free(current);
		gamedata = NULL;
		return 0;
	}
	
	while(current != NULL){
		if (current->next != NULL){
			/* There's another element, so free() this one and
			    then move on to the next */
			next = current->next;
			if (verbose){
				printf("%s.%d\t Freeing gamedata list object [%s]\n", __FILE__, __LINE__, current->next->name);	
			}
			free(current);
			current = next;
		} else {
			/* There are no more elements linked */
			free(current);
			current = NULL;
			return 0;
		}
	}
	return 0;
}

int removeImagefile(imagefile_t *imagefile){
	/* Remove all nodes of a given imagefile list */
	
	imagefile_t * current = imagefile;
	imagefile_t * next = NULL;
	
	if (current != NULL){
	
		if (current->next == NULL){
			//if (verbose){
			//	printf("%s.%d\t Freeing imagefile single object\n", __FILE__, __LINE__);	
			//}
			free(current);
			imagefile = NULL;
			return 0;
		}
		
		while(current != NULL){
			if (current->next != NULL){
				/* There's another element, so free() this one and
					then move on to the next */
				next = current->next;
				//if (verbose){
				//	printf("%s.%d\t Freeing imagefile list object [%s]\n", __FILE__, __LINE__, current->next->filename);	
				//}
				free(current);
				current = next;
			} else {
				/* There are no more elements linked */
				free(current);
				current = NULL;
				return 0;
			}
		}
	}
	return 0;
}

int sortGamedata(gamedata_t *gamedata, int verbose){
	// Sort the list of game data objects by name
	// This is bubble sort, so it's reasonably slow, but 
	// simple to implement.
	
	int swapped;
	int compare;
	
	gamedata_t *gdata1 = NULL;
	gamedata_t *gdata2 = NULL;
	
	/* Nothing more after this point, consider it sorted */
	if (gamedata->next == NULL){
		return 0;
	}
	
	do {
		swapped = 0;
		gdata1 = gamedata;
		while (gdata1->next != gdata2){
			compare = strcmp(gdata1->name, gdata1->next->name);
			if (compare > 0){
				/* swap objects */
				swapGamedata(gdata1, gdata1->next);
				swapped = 1;
			}
			gdata1 = gdata1->next;
		}
		gdata2 = gdata1;
	}
	while (swapped);
	return 0;
}

int swapGamedata(gamedata_t *gamedata1, gamedata_t *gamedata2){
	/* swap one game object with another */
	
	gamedata_t *gdata_temp = NULL;
	gdata_temp = (gamedata_t *) malloc(sizeof(gamedata_t));
	
	/* keep temp store of gamedata1 */
	gdata_temp->drive = gamedata1->drive;
	strcpy(gdata_temp->path, gamedata1->path);
	strcpy(gdata_temp->name, gamedata1->name);
	gdata_temp->has_dat = gamedata1->has_dat;
	
	/* swap a with b */
	gamedata1->drive = gamedata2->drive;
	strcpy(gamedata1->path, gamedata2->path);
	strcpy(gamedata1->name, gamedata2->name);
	gamedata1->has_dat = gamedata2->has_dat;
	
	/* swap b with temp */
	gamedata2->drive = gdata_temp->drive;
	strcpy(gamedata2->path, gdata_temp->path);
	strcpy(gamedata2->name, gdata_temp->name);
	gamedata2->has_dat = gdata_temp->has_dat;
	
	/* Free up temp store */
	free(gdata_temp);
	return 0;
}

static int launchdatHandler(void* user, const char* section, const char* name, const char* value){
	/* Based on reference implementation of inih parser:
	    https://github.com/benhoyt/inih
	    */
	    
	launchdat_t* launchdat = (launchdat_t*)user;
		
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("default", "name")){
		strcpy(launchdat->realname, value);
	} else if (MATCH("default", "genre")){
		strcpy(launchdat->genre, value);
	} else if (MATCH("default", "developer")){
		strcpy(launchdat->publisher, value);
	} else if (MATCH("default", "publisher")){
		strcpy(launchdat->publisher, value);
	} else if (MATCH("default", "year")){
		launchdat->year = atoi(value);
	} else if (MATCH("default", "start")){
		strcpy(launchdat->start, value);
	} else if (MATCH("default", "images")){
		strcpy(launchdat->images, value);
	} else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

void launchdataDefaults(launchdat_t *launchdat){
	/* Set some defaults, in case various lines arent there */
	
	//strcpy(launchdat->realname, "Unknown");
	memset(launchdat->realname, '\0', strlen(launchdat->realname));
	//strcpy(launchdat->genre, DEFAULT_GENRE);
	memset(launchdat->genre, '\0', strlen(launchdat->genre));
	launchdat->year = DEFAULT_YEAR;
	//strcpy(launchdat->publisher, DEFAULT_PUBLISHER);
	memset(launchdat->publisher, '\0', strlen(launchdat->publisher));
	//strcpy(launchdat->developer, DEFAULT_DEVELOPER);
	memset(launchdat->developer, '\0', strlen(launchdat->developer));
	//strcpy(launchdat->start, DEFAULT_START);
	memset(launchdat->start, '\0', strlen(launchdat->start));
	memset(launchdat->images, '\0', strlen(launchdat->images));
}

void configDefaults(config_t *config){
	/* Set some defaults, in case various lines arent there */
	
	config->verbose = 0;
	memset(config->dirs, '\0', strlen(config->dirs));
	config->save = 0;
	config->dir = NULL;
}

int getLaunchdata(gamedata_t *gamedata, launchdat_t *launchdat){
	/* load and return a launch.dat from from disk, for a given gamedata object */
	
	char filepath[65];
	
	if (gamedata->has_dat != 1){
		return -1;
	}
	
	strcpy(filepath, gamedata->path);
	strcat(filepath, "\\");
	strcat(filepath, GAMEDAT);
	
	launchdataDefaults(launchdat);
	if (ini_parse(filepath, launchdatHandler, launchdat) < 0) {
		//printf("%s.%d\t Cannot load %s\n", __FILE__, __LINE__, filepath);
		return -1;
	} else {
		return 0;
	}
}

static int configHandler(void* user, const char* section, const char* name, const char* value){
	/* Based on reference implementation of inih parser:
	    https://github.com/benhoyt/inih
	    */
	    
	config_t* config = (config_t*)user;
		
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("default", "verbose")){
		config->verbose =  atoi(value);
	} else if (MATCH("default", "gamedirs")){
		strcpy(config->dirs, value);
	} else if (MATCH("default", "savedirs")){
		config->save =  atoi(value);
	} else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

int getIni(config_t *config, int verbose){
	/* Load the main ini file for the application */
	
	int status;
	char my_drive;
	char my_drive_letter;
	char my_dir[DIR_BUFFER_SIZE];
	char my_path[DIR_BUFFER_SIZE];
	
	if (verbose){
		printf("%s.%d\t Initialising buffers\n", __FILE__, __LINE__);
	}
	memset(my_dir, '\0', sizeof(my_dir));
	memset(my_path, '\0', sizeof(my_path));
	my_drive = _dos_curdrv();
	my_drive_letter = drvNumToLetter(my_drive);
	
	if (verbose){
		printf("%s.%d\t Current drive: %c\n", __FILE__, __LINE__, my_drive_letter);
	}
	status = _dos_curdir((my_drive + 1), my_dir);
	if (status < 0){
		if (verbose){
			printf("%s.%d\t Unable to determine current directory\n", __FILE__, __LINE__);
		}
		return -1;
	} else {
		if (verbose){
			printf("%s.%d\t Current directory: %s\n", __FILE__, __LINE__, my_dir);
		}
	}
	my_path[0] = my_drive_letter;
	strcat(my_path, ":\\");
	
	if (strlen(my_dir) > 0){
		strcat(my_path, my_dir);
		strcat(my_path, "\\");
	}	
	strcat(my_path, INIFILE);
	
	configDefaults(config);
	
	if (verbose){
		printf("%s.%d\t Calling parser\n", __FILE__, __LINE__);
	}
	if (ini_parse(my_path, configHandler, config) < 0) {
		printf("%s.%d\t Cannot load %s\n", __FILE__, __LINE__, my_path);
		return -1;
	} else {
		return 0;
	}
}

int getImageList(launchdat_t *launchdat, imagefile_t *imagefile){
	/* build a list of images as defined in launch.dat */
	/* Should only ever be called ONCE for a given imagefile!!! */
	
	char *p;
	char buffer[IMAGE_BUFFER_SIZE];
	int found;	// Counter for number of found images
	found = 0;
	
	if (launchdat->images != NULL){
		strcpy(buffer, launchdat->images);
		p = strtok(buffer, ",");
		while (p != NULL){
			found++;
			imagefile = getLastImage(imagefile);
			imagefile->next = (imagefile_t *) malloc(sizeof(imagefile_t));
			strcpy(imagefile->next->filename, p);
			imagefile->next->next = NULL;
			if (found >= MAX_IMAGES){
				return found;
			}
			p = strtok(NULL, ",");
		}
		return found;
	} else {
		return -1;	
	}	
}

int getDirList(config_t *config, gamedir_t *gamedir, int verbose){
	/* build a list of game search directoes as defined in launcher.ini */
	/* Should only ever be called ONCE at startup!!! */
	
	char *p;
	char buffer[1024];
	int found;	// Counter for number of found game search directories
	found = 0;
	
	if (config->dirs != NULL){
		strcpy(buffer, config->dirs);
		p = strtok(buffer, ",");
		while (p != NULL){
			if (isDir(p)){
				if (verbose){
					printf("%s.%d\t Game search path %s\n", __FILE__, __LINE__, p);	
				}
				found++;
				gamedir = getLastGameDir(gamedir);
				gamedir->next = (gamedir_t *) malloc(sizeof(gamedir_t));
				strcpy(gamedir->next->path, p);
				gamedir->next->next = NULL;
				
				// If this was the first gamedir found, add it as the head item to the config data object
				if (found == 1){
					config->dir = gamedir;	
				}
				if (found >= MAX_DIRS){
					if (verbose){
						printf("%s.%d\t Hit game search path limit [%d]\n", __FILE__, __LINE__, MAX_IMAGES);	
					}
					break;
				}
			} else {
				if (verbose){
					printf("%s.%d\t Game search path %s not found, skipping\n", __FILE__, __LINE__, p);
				}
			}
			p = strtok(NULL, ",");
		}
		if (verbose){
			printf("%s.%d\t Found [%d] game search paths\n", __FILE__, __LINE__, found);	
		}
	} else {
		if (verbose){
			printf("%s.%d\t No game search paths found\n", __FILE__, __LINE__);	
		}
		return -1;	
	}	
	return found;
}