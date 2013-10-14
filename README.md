# SQUABBLE

An interactive word-puzzle board game with AI opponent.

The "share" directory has sample configurations, but you may want to create your own files based on the instructions in "note" in order to match preferences for the board layout.  Squabble will soon read these files first from ~/.config/squabble/{dict,tiles,bonus} to allow for easier customization.

### Installation

Ensure the relevant dependencies are installed: git cairo libx11.  The development packages (-dev) will need to be installed on distros with split packages.

```bash
git clone http://github.com/TrilbyWhite/squabble.git
cd squabble
make
sudo make install
```

Archlinux users may use the provided PKGBUILD.  Comparable build scripts for other distros can be submitted via email for inclusion here.

### Controls

- Move tiles with the mouse
- Hit enter to complete a play
- Press 'q' at anytime to quit
- More to come soon


### Todo / Bugs

- Add end of game ... as is, tiles just run out
- Allow user to trade tiles (need to shuffle before replacement)
	- Make tile-bag tiles movable
- Add "wildcard" tiles
- Add config dir for bonuses, tiles, and dictionary
- Add rc file for difficulty settings, player name, etc
	- Allow use of user name
	- Give names to AI difficulty levels?
- Allow edits to (user) dictionary
	- Requires writing of dictionary on close
- Much code cleanup needed





