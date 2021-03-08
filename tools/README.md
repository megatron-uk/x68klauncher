# Tools

----

## metadata.py

A tool to scrape metadata from Mobygames or from Launchbox, using the output from `launcher.X` when run in 'save' mode.

Place the resulting 'launcher.txt' file in this directory and run metadata.py to download metadata and generate 'launch.dat' and artwork images.

#### Mobygames

*Note:* For Mobygames, you will need a valid API key (which is documented here: https://www.mobygames.com/info/api), saved in a file called `mobygames.py` in this directory. The content of the file should look like this:

```
API_KEY = "abc123qertyuiop"
```
#### Launchbox

*Note:* For Launchbox you will need a copy of Metadata.xml, which can be obtained here: http://gamesdb.launchbox-app.com/Metadata.zip - download the file, unzip and place in this folder.


#### Enabling metadata providers

Edit the `metadata.py` script and enable either Mobygames or Launchbox mode by setting either of the following to True:

```
USE_MOBYGAMES = 
USE_LAUNCHBOX = 
```