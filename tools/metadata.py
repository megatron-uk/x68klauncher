#!/usr/bin/env python3

from lxml import etree
import os
import sys
import time
import requests
import subprocess

from mobygames import API_KEY

search_payload = {
	'api_key' : API_KEY,
	'format' : 'normal',
	'title' : 'chorensha'
}

detail_payload = {
	'api_key' : API_KEY,
	'format' : 'normal',
}

SEARCH_URL 		= "https://api.mobygames.com/v1/games"
DETAIL_URL 		= "https://api.mobygames.com/v1/games/"
PLATFORM_URL		= "https://api.mobygames.com/v1/games/"

MY_PLATFORM 	= "Sharp X68000"

# Specific genre tags as returned by mobygames, we don't want to be choosing from '3/4 perspective' or 'helicopter game'...
# Just things like: 'arcade', 'platformer', 'rpg', 'shooter' etc.
MY_GENRE_CATEGORIES = ["Basic Genres", "Gameplay"]

# What mobygames labels certain types of company as
ROLE_DEVELOPER = "Developed by"
ROLE_PUBLISHER = "Published by"
ROLE_PORTER = "Ported by"

# Name of the output metadata file
METADATA_FILE = "launch.dat"
IMAGE_FILE_PREFIX = "launch"
OUTPUT_DIR = "out"
LAUNCHBOX_XML = "Metadata.xml"
LAUNCHBOX_IMAGE_URL = "https://images.launchbox-app.com/"

# Which API to use
USE_MOBYGAMES = False
USE_LAUNCHBOX = True

if os.path.exists(LAUNCHBOX_XML):
	print("Found %s, you can use the Launchbox API if needed" % LAUNCHBOX_XML)
	if USE_LAUNCHBOX:
		print("Parsing XML...")
		tree = etree.parse(LAUNCHBOX_XML).getroot()
		print("Done")
else:
	print("%s missing, you cannot use the Launchbox API" % LAUNCHBOX_XML)
	USE_LAUNCHBOX = False

if USE_MOBYGAMES or USE_LAUNCHBOX:
	interactive = True
else:
	interactive = False

#########################################
#
# Common functions
#
#########################################

def gameHasImages(gamedata):

	game_output_path = OUTPUT_DIR + "/" + gamedata['game_drive'] + gamedata['unix_directory_path']

	if os.path.exists(game_output_path + "/" + IMAGE_FILE_PREFIX + "01.bmp"):
		return True
	else:
		return False

def gameHasMetadata(gamedata):
	""" Returns true if a metadata file already exists in the output folder for a given game """
	
	game_output_path = OUTPUT_DIR + "/" + gamedata['game_drive'] + gamedata['unix_directory_path'] 
	
	if os.path.exists(game_output_path + "/" + METADATA_FILE):
		return True
	else:
		return False
	
def createMetadataPath(gamedata):
	""" Create the output directory for a games metadata file """
	
	game_output_path = OUTPUT_DIR + "/" + gamedata['game_drive'] + gamedata['unix_directory_path'] 
	
	return os.makedirs(game_output_path, exist_ok=True)

def showMetadata(selected_game, game_details, selected_company, selected_genre, selected_images):
	
	print("")
	print("Step 9 - Confirm metadata...")
	metadata = ""
	metadata += "[default]\n"
	metadata += "name=%s\n" % selected_game['title']
	metadata += "year=%s\n" % game_details['first_release_date'].split("-")[0]
	metadata += "genre=%s\n" % selected_genre['genre_name']
	if selected_company:
		for c in selected_company:
			if c['role'] == ROLE_PUBLISHER:
				metadata += "publisher=%s\n" % c['company_name']
			if c['role'] == ROLE_DEVELOPER:
				metadata += "developer=%s\n" % c['company_name']
			if c['role'] == ROLE_PORTER:
				metadata += "ported=%s\n" % c['company_name']
	metadata += "source=Mobygames.com\n"
	print(metadata)
	if selected_images:
		image_names = ""
		for i in selected_images:
			print("image=%s\n" % i['image'])
			
	return metadata

def showMetadataXML(selected_game, selected_company, selected_genre, selected_images):
	
	print("")
	print("Step 5 - Confirm metadata...")
	metadata = ""
	metadata += "[default]\n"
	metadata += "name=%s\n" % selected_game['title']
	metadata += "year=%s\n" % selected_game['release']
	metadata += "genre=%s\n" % selected_genre
	if selected_company:
		for c in selected_company:
			if c['role'] == ROLE_PUBLISHER:
				metadata += "publisher=%s\n" % c['company_name']
			if c['role'] == ROLE_DEVELOPER:
				metadata += "developer=%s\n" % c['company_name']
			if c['role'] == ROLE_PORTER:
				metadata += "ported=%s\n" % c['company_name']
	metadata += "source=gamesdb.launchbox-app.com\n"
	print(metadata)
	if selected_images:
		image_names = ""
		for i in selected_images:
			print("image=%s\n" % i['image'])
			
	return metadata

def generatePossibleName(gamename):
	""" Generate a possible game name from the directory name. """
	
	# We follow these rules:
	# 1. Insert a space before every uppercase letter
	# 2. Insert a space before every number
	
	new_gamename = ""
	for c in gamename:
		if c.isupper():
			new_gamename += " " + c
		elif c.isdigit():
			new_gamename += " " + c
		elif c in ["_"]:
			pass
		else:
			new_gamename += c
	
	return new_gamename.lstrip().rstrip()
	
######################################
#
# Mobygames API
#
######################################
			
def lookupGameImages(game_id = None, platform_id = None):
	""" Lookup game images information """
	
	ids = {}
	i = 0
	u = PLATFORM_URL + str(game_id) + "/platforms/" + str(platform_id) + "/screenshots"
	print("")
	print("Step 7 - Retrieve possible screenshots...")
	time.sleep(1)
	r = requests.get(u, params=detail_payload)
	if (r.status_code == 200):
		try:
			data = r.json()
			if 'screenshots' in data.keys():
				print("Done")
				return data['screenshots']
			else:
				print("No data returned")
				return False
		except Exception as e:
			print("Error: %s" % e)
			return False
	else:
		print("Error, received code %s from server" % response.status_code)
		return False

def lookupGamePlatforms(game_id = None, platform_id = None):
	""" Lookup game platform information """
	
	ids = {}
	i = 0
	u = PLATFORM_URL + str(game_id) + "/platforms/" + str(platform_id)
	print("")
	print("Step 4 - Retrieve possible platform releases...")
	time.sleep(1)
	r = requests.get(u, params=detail_payload)
	if (r.status_code == 200):
		try:
			data = r.json()
			if 'releases' in data.keys():
				print("Done")
				return data
			else:
				print("No data returned")
				return False
		except Exception as e:
			print("Error: %s" % e)
			return False
	else:
		print("Error, received code %s from server" % response.status_code)
		return False

def lookupGames(gamedata = None):
	""" Extract game data information from a JSON response from Mobygames """

	print("")
	print("Step 1 - Retrieve matching games...")
	time.sleep(1)
	r = requests.get(SEARCH_URL, params=search_payload)
	if (r.status_code == 200):
		try:
			data = r.json()
			if 'games' in data.keys():
				print("Done")
				return data['games']
			else:
				print("No data returned")
				return False
		except Exception as e:
			print("Error: %s" % e)
			return False
	else:
		print("Error, received code %s from server" % response.status_code)
		return False

def lookupGameDetails(game = None):
	""" Look up information on a specific game """
	
	u = DETAIL_URL + str(game['game_id'])
	print("")
	print("Step 2 - Retrieve details...")
	time.sleep(1)
	r = requests.get(u, params=detail_payload)
	if (r.status_code == 200):
		data = r.json()
		try:
			if ('game_id' in data.keys()) and ('platforms' in data.keys()):
				print("Done")
				return data
			else:
				print("No data returned")
				return False
		except Exception as e:
			print("Error: %s" % e)
			return False
	else:
		print("Error, received code %s from server" % response.status_code)
		return False

def selectGameCompanies(gameplatforms):
	i = 0
	companies = {}
	print("")
	print("Step 5 - Confirm company/developer/publisher and release version...")
	for r in gameplatforms['releases']:
		for c in r['companies']:
			i+=1
			companies[str(i)] = c
			print("%02s. | Company: %s | Role: %s | Year: %s" % (i, c['company_name'], c['role'], r['release_date']))

	# Choose one from the list
	if (len(companies.keys()) > 0):
		print("")
		print("Enter either a single number %s, multiple numbers seperated by spaces, or enter to skip these choices" % list(companies.keys()))
		ch = input()
		ch = str(ch)
		return_companies = []
		for c in ch.split(" "):
			if c in companies.keys():
				print("Added company %s." % c)
				return_companies.append(companies[c])
		if len(return_companies) > 0:
			return return_companies
		else:
			print("No companies selected")
			print("")
			return False
	else:
		print("No valid companies found in server response, skipping this game.")
		print("")
		return False

def selectGenre(gamedetails):
	
	i = 0
	genres = {}
	print("")
	print("Step 6 - Select game genre...")
		
	for g in gamedetails['genres']:
		if g['genre_category'] in MY_GENRE_CATEGORIES:
			i+=1
			genres[str(i)] = g
			print("%2d. | Genre: %s" % (i, g['genre_name']))
	
	# Choose one from the list
	if (len(genres.keys()) > 0):
		print("")
		print("Enter a number %s, or enter to skip these choices" % list(genres.keys()))
		c = input()
		c = str(c)
		if c in genres.keys():
			return genres[c]
		else:
			print("Not a valid ID, skipping this game.")
			print("")
			return False
	else:
		print("No valid genres found in server response, skipping this game.")
		print("")
		return False
	
def selectGameImages(gameimages):
	
	i = 0
	images = {}
	print("")
	print("Step 8 - Choose screenshots...")
	for s in gameimages:
		i+=1
		images[str(i)] = s
		print("%02s. | Caption: %s" % (i, s['caption']))
		print("%02s. | Image: %s" % (i, s['image']))

	# Choose one or more from the list
	if (len(images.keys()) > 0):
		print("")
		print("Enter either a single number %s, multiple numbers seperated by spaces, or enter to skip these choices" % list(images.keys()))
		print("NOTE: The order you enter the numbers is the order in which they will be shown in the artwork browser.")
		print("NOTE: Tip - you probably want the title screen first!")
		ch = input()
		ch = str(ch)
		return_images = []
		for c in ch.split(" "):
			if c in images.keys():
				print("Added image %s." % c)
				return_images.append(images[c])
		if len(return_images) > 0:
			return return_images
		else:
			print("No images selected")
			print("")
			return False
	else:
		print("No valid images found in server response, skipping this game.")
		print("")
		return False
	
def selectGameDetails(gamedetails):
	
	i = 0
	platforms = {}
	print("")
	print("Step 3 - Confirm initial game details & select platform...")
	print("Name	: %s" % gamedetails['title'])
	print("URL	: %s" % gamedetails['moby_url'])
	for p in gamedetails['platforms']:
		i+=1
		platforms[str(i)] = p
		print("%02s. | Platform: %s (%s)" % (i, p['platform_name'], p['first_release_date']))

	# Choose one from the list
	if (len(platforms.keys()) > 0):
		print("")
		print("Enter a number %s, or enter to skip these choices" % list(platforms.keys()))
		c = input()
		c = str(c)
		if c in platforms.keys():
			return platforms[c]
		else:
			print("Not a valid ID, skipping this game.")
			print("")
			return False
	else:
		print("No valid platforms found in server response, skipping this game.")
		print("")
		return False
	
def selectGame(games):
	""" Select one of the returned games as a match """
	
	i = 0
	game_options = {}
	
	print("")
	print("Step 2 - Choose from an initial list of possible games...")
		
	# Display a list of games
	for g in games:
		for p in g['platforms']:
			if p['platform_name'] == MY_PLATFORM:
				has_my_platform = True
				i+=1
				game_options[str(i)] = g
				platforms = ""
				has_my_platform = False
				for p in g['platforms']:
					platforms = platforms + ", " + p['platform_name']
				print("%2s. |  ID: %s | Title: %s |  Platforms: %s" % (i, g['game_id'], g['title'], platforms))

	# Choose one from the list
	if (len(game_options.keys()) > 0):
		print("")
		print("Enter a number %s, or enter to skip these choices" % list(game_options.keys()))
		c = input()
		c = str(c)
		if c in game_options.keys():
			return game_options[c]
		else:
			print("Not a valid ID, skipping this game.")
			print("")
			return False
	else:
		print("No valid games found in server response, skipping this game.")
		print("")
		return False

#######################################
#
# Launchbox XML API
#
#######################################

def lookupGamesXML(gamename):
	""" Search for matching games in the launchbox XML """

	g = None
	gamedatas = []

	print("")
	print("Step 1 - Finding matches in metadata XML...")
	games = tree.xpath("//Game")
	for g in games:
		if gamename.upper() in g.findtext('Name').upper():
			gamedata = {
				'title' 		: g.find('./Name').text,
				'game_id'	: g.find('./DatabaseID').text,
				'genre'		: None,
				'publisher'	: None,
				'developer'	: None,
				'platform'	: g.find('./Platform').text,
				'release'		: None
			}
			if g.find('./ReleaseYear') is not None:
				gamedata['release'] = g.find('./ReleaseYear').text
			if g.find('./Developer') is not None:
				gamedata['developer'] = g.find('./Developer').text
			if g.find('./Publisher') is not None:
				gamedata['publisher'] = g.find('./Publisher').text
			if g.find('./Genres') is not None:
				gamedata['genre'] = g.find('./Genres').text
			if g.find('./ReleaseDate') is not None:
				gamedata['release'] = g.find('./ReleaseDate').text.split("-")[0]
				
			if gamedata['platform'] == MY_PLATFORM:
				gamedatas.append(gamedata)
			
	if len(gamedatas) > 0:
		print("Done")
		return gamedatas
	else:
		print("No valid games found in XML, skipping this game.")
		print("")
		return False

def lookupGameImagesXML(selected_gameXML):
	""" Search for matching game images in the launchbox XML """

	g = None
	gameimages = []

	print("")
	print("Step 3 - Finding images in metadata XML...")
	gameimage = tree.xpath("//GameImage")
	for i in gameimage:
		if selected_gameXML['game_id'] == i.findtext('DatabaseID'):
			image = {
				'caption' 	: i.find('./Type').text,
				'image'	: LAUNCHBOX_IMAGE_URL + i.find('./FileName').text
			}
			gameimages.append(image)
			
	if len(gameimages) > 0:
		print("Done")
		return gameimages
	else:
		print("No valid images found in XML, skipping this game.")
		print("")
		return False

def selectGameImagesXML(gameimagesXML):
	""" Select one or more images """
		
	i = 0
	images = {}
	print("")
	print("Step 4 - Choose screenshots...")
	for s in gameimagesXML:
		i+=1
		images[str(i)] = s
		print("%02s. | Caption: %s" % (i, s['caption']))
		print("%02s. | Image: %s" % (i, s['image']))

	# Choose one or more from the list
	if (len(images.keys()) > 0):
		print("")
		print("Enter either a single number %s, multiple numbers seperated by spaces, or enter to skip these choices" % list(images.keys()))
		print("NOTE: The order you enter the numbers is the order in which they will be shown in the artwork browser.")
		print("NOTE: Tip - you probably want the title screen first!")
		ch = input()
		ch = str(ch)
		return_images = []
		for c in ch.split(" "):
			if c in images.keys():
				print("Added image %s." % c)
				return_images.append(images[c])
		if len(return_images) > 0:
			return return_images
		else:
			print("No images selected")
			print("")
			return False
	else:
		print("No valid images found in XML, skipping this game.")
		print("")
		return False

def selectGameXML(gamedataXML):
	""" Select a game from one returned from the Launchbox XML """
	
	i = 0
	game_options = {}
	
	print("")
	print("Step 2 - Choose from an initial list of possible games...")
		
	# Display a list of games
	for g in gamedataXML:
		i+=1
		game_options[str(i)] = g
		platforms = ""
		has_my_platform = False
		print("%2s. |  ID: %s | Title: %s |  Platform: %s | Year: %s | Genre: %s" % (i, g['game_id'], g['title'], g['platform'], g['release'], g['genre']))

	# Choose one from the list
	if (len(game_options.keys()) > 0):
		print("")
		print("Enter a number %s, or enter to skip these choices" % list(game_options.keys()))
		c = input()
		c = str(c)
		if c in game_options.keys():
			return game_options[c]
		else:
			print("Not a valid ID, skipping this game.")
			print("")
			return False
	else:
		print("No valid games found in XML, skipping this game.")
		print("")
		return False
	
def selectGenreXML(selected_gameXML):
	""" Select from a list of genres for the game """
	
	i = 0
	game_genres = {}
	
	print("")
	print("Step 2 - Select a genre...")
	if selected_gameXML['genre'] is not None:
		genres = selected_gameXML['genre'].split(';')
		for g in genres:
			i+=1
			g = g.lstrip().rstrip()
			game_genres[str(i)] = g
			print("%2s. | Genre: %s" % (i, g))
			
		# Choose one from the list
		if (len(game_genres.keys()) > 0):
			print("")
			print("Enter a number %s, or enter to skip these choices" % list(game_genres.keys()))
			c = input()
			c = str(c)
			if c in game_genres.keys():
				return game_genres[c]
			else:
				print("Not a valid ID, skipping this game.")
				print("")
				return False
		else:
			print("No genre information found in XML, skipping this game.")
			print("")
			return False
	else:
		print("No genre information found in XML, skipping this game.")
		print("")
		return False
	
def selectCompanyXML(selected_gameXML):
	""" Select from a list of publishers and developers for the game """
	
	i = 0
	game_companies = {}
	
	print("")
	print("Step 2 - Select a company...")
	if selected_gameXML['developer'] is not None:
		i+=1
		c = { 'role' : ROLE_DEVELOPER, 'company_name' : selected_gameXML['developer'] }
		game_companies[str(i)] = c
		print("%2s. | Developer: %s" % (i, selected_gameXML['developer']))
	if selected_gameXML['publisher'] is not None:
		i+=1
		c = { 'role' : ROLE_PUBLISHER, 'company_name' : selected_gameXML['publisher'] }
		game_companies[str(i)] = c
		print("%2s. | Publisher: %s" % (i, selected_gameXML['publisher']))
		
	if len(game_companies) > 0:
		print("")
		print("Enter either a single number %s, multiple numbers seperated by spaces, or enter to skip these choices" % list(game_companies.keys()))
		ch = input()
		ch = str(ch)
		return_companies = []
		for c in ch.split(" "):
			if c in game_companies.keys():
				print("Added company %s." % c)
				return_companies.append(game_companies[c])
		if len(return_companies) > 0:
			return return_companies
		else:
			print("No companies selected")
			print("")
			return False
	else:
		print("No company information found in XML, skipping this game.")
		print("")
		return False
	
#######################################
#
# Main function
#
#######################################

def main():
	titles = 0
	titles_with_metadata = 0
	titles_with_images = 0
	f = open("launcher.txt", "r")
	for l in f:
		titles += 1
		
		l = l.rstrip().replace('\n', '')
		
		drive = l.split(':')[0]
		game_subdir = l.split('\\')[-1]
		game_directory_path = l.split(':')[1]
		possible_title = generatePossibleName(game_subdir)
		
		gamedata = {
			'full_path' : l,	
			'possible_title' : possible_title,
			'game_drive' : drive,
			'game_subdir' : game_subdir,
			'game_directory_path' : game_directory_path,
			'unix_directory_path' : game_directory_path.replace('\\', '/')
		}
	
		print("======================================")
		print("")
		print("Title:		%s" % gamedata['possible_title'])
		print("Subdir:		%s" % gamedata['game_subdir'])
		print("Drive:		%s" % gamedata['game_drive'])
		print("Path:		%s" % gamedata['unix_directory_path'])
		print("")
		
		if gameHasImages(gamedata):
			print("Images already exists")
			titles_with_images += 1
		
		if gameHasMetadata(gamedata):
			print("Metadata file already exists")
			titles_with_metadata += 1
		else:
			createMetadataPath(gamedata)
			
			if interactive:
			
				print("Use [%s] for search? (y/n)" % gamedata['possible_title'])
				c = input()
				if (c in ['y', 'Y']):
					gamename = gamedata['possible_title']
				elif (c in ['n', 'N']):
					print("Ok, enter alternative game name:")
					gamename = input()
				else:
					print("Not a valid input, skipping to next game.")
					gamename = False
				
				
				
				# Options which come back from Mobygames
				game = False
				gamedetails = False
				gameplatform = False
				gamegenres = False
				gameimages = False
				gamecompanies = False
				
				# Selections
				selected_platform = False
				selected_company = False
				selected_genre = False
				selected_images = False
					
				if USE_MOBYGAMES:
					print("Trying Mobygames...")
					if gamename:
						search_payload['title'] = gamename
						time.sleep(1)
						games = lookupGames(gamedata)
						if games:
							selected_game = selectGame(games)
							if selected_game:
								# Load the basic game details
								selected_game_details = lookupGameDetails(game = selected_game)
								
								# Select a specific platform version of this game
								game_details = selectGameDetails(selected_game_details)
								
								if game_details:
									# Load the list of companies responsible for this platform release
									gamecompanies = lookupGamePlatforms(selected_game['game_id'], game_details['platform_id'])
									
								if gamecompanies:
									# Select the companies
									selected_company = selectGameCompanies(gamecompanies)
									
								if selected_game:
									# Select the genre
									selected_genre = selectGenre(selected_game_details)
									
								#if game_details:
									# Load the list of images for this platform
								#	gameimages = lookupGameImages(selected_game['game_id'], game_details['platform_id'])
									
								#if gameimages:
									# Select the images
								#	selected_images = selectGameImages(gameimages)
									
								metadata = showMetadata(selected_game, game_details, selected_company, selected_genre, selected_images)
								print("")
								print("Are you happy with this data? (y/n)")
								c = input()
								if (c in ['y', 'Y']):
									print("")
									print("Step 10 - Starting image download...")
									i = 0
									failures = 0
									image_names = ""
									game_output_path = OUTPUT_DIR + "/" + gamedata['game_drive'] + gamedata['unix_directory_path']
									if selected_images:
										for img in selected_images:
											i+=1
											image_name = "%s%02d" % (IMAGE_FILE_PREFIX, i)
											
											print("Downloading %s of %s" % (i, len(selected_images)))
											r = requests.get(img['image'])
											if (r.status_code == 200):
												# Write downloaded image to disk
												open(game_output_path + "/" + image_name, 'wb').write(r.content)
				
												# Use imagemagick to convert from web format to 16bit, 256x256, BMP
												cmd = """cd %s  && convert -resize 256x256 -type truecolor -define bmp:subtype=RGB565 %s %s.bmp && rm %s""" % (game_output_path, image_name, image_name, image_name)
												print("Converting %s of %s" % (i, len(selected_images)))
												status = subprocess.run(cmd, shell=True, check=True)
												if status.returncode == 0:
													image_names += image_name + ".bmp,"
												else:
													print("Image %s failed to convert" % i)
													failures += 1
				
											else:
												print("Image %s failed to download" % i)
												failures += 1
									if (failures == 0):
										# Write  metadata
										if selected_images:
											metadata += "images=%s" % image_names
										open(game_output_path + "/" + METADATA_FILE, "w").write(metadata)
										print("Wrote metadata!")
										titles_with_metadata += 1
									else:
										print("Not writing metadata, at least one download failed, skipping to next game")
								else:
									print("Skipping to next game")
				if USE_LAUNCHBOX:
					print("Trying Launchbox metadata.xml")
					if gamename:
						games = lookupGamesXML(gamename)
						if games:
							selected_game = selectGameXML(games)
							if selected_game:
								selected_genre = selectGenreXML(selected_game)
								if selected_game:
									selected_company = selectCompanyXML(selected_game)
								if selected_game:
									gameimages = lookupGameImagesXML(selected_game)
								if gameimages:
									selected_images = selectGameImagesXML(gameimages)
								
								metadata = showMetadataXML(selected_game, selected_company, selected_genre, selected_images)
								print("")
								print("Are you happy with this data? (y/n)")
								c = input()
								if (c in ['y', 'Y']):
									print("")
									print("Step 6 - Starting image download...")
									i = 0
									failures = 0
									image_names = ""
									game_output_path = OUTPUT_DIR + "/" + gamedata['game_drive'] + gamedata['unix_directory_path']
									if selected_images:
										for img in selected_images:
											i+=1
											image_name = "%s%02d" % (IMAGE_FILE_PREFIX, i)
											
											print("Downloading %s of %s" % (i, len(selected_images)))
											r = requests.get(img['image'])
											if (r.status_code == 200):
												# Write downloaded image to disk
												open(game_output_path + "/" + image_name, 'wb').write(r.content)
				
												# Use imagemagick to convert from web format to 16bit, 256x256, BMP
												cmd = """cd "%s"  && convert -resize 256x256 -type truecolor -define bmp:subtype=RGB565 %s %s.bmp && rm %s""" % (game_output_path, image_name, image_name, image_name)
												print("Converting %s of %s" % (i, len(selected_images)))
												status = subprocess.run(cmd, shell=True, check=True)
												if status.returncode == 0:
													image_names += image_name + ".bmp,"
												else:
													print("Image %s failed to convert" % i)
													failures += 1
				
											else:
												print("Image %s failed to download" % i)
												failures += 1
									if (failures == 0):
										# Write  metadata
										if selected_images:
											metadata += "images=%s" % image_names
										open(game_output_path + "/" + METADATA_FILE, "w").write(metadata)
										print("Wrote metadata!")
										titles_with_metadata += 1
									else:
										print("Not writing metadata, at least one download failed, skipping to next game")
								else:
									print("Skipping to next game")
							else:
								print("Skipping to next game")
	f.close()
	print("")
	print("==============================")
	print("")
	print("Total titles		: %s" % titles)
	print("+ metadata		: %s" % titles_with_metadata)
	print("+ images		: %s" % titles_with_images)
	print("")
	print("==============================")
	sys.exit(0)

if __name__ == "__main__":
	main()
