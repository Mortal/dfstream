Dependencies
============

* GNU/Linux
* A recent C++ compiler with C++11 support (clang specified in Makefile).
* Python2 with Twisted for the stream multiplexer
* Ncurses built with wide character support (ncursesw)

Usage
=====

The starting point is Dwarf Fortress, a multiplatform game that in particular
runs in GNU/Linux under X11. Its graphical interface is grid and tile based;
each tile is 8 pixels wide and 12 pixels tall and uses a particular foreground
color and background color (with no anti-aliasing). Each tile comes from a
spriteset of 256 tiles.

This repository contains several programs that chain together to make streaming
of tile-based games possible.

First, `patterntrainer` accepts as input several lines, all of which have the
same length, all consisting of zeroes and ones, and outputs a binary decision
tree in C that decides which sprite a given array of bools corresponds to, on
the assumption that it corresponds to one of the tiles in the input.

Second, `ffmpeg` and its `x11grab` input source grabs the Dwarf Fortress screen
and outputs it in PPM format (using the included `x11grab.sh` helper scripts).

Third, `compressor` reads the PPM stream and uses the binary decision tree from
`patterntrainer` to decode the PPM stream to a stream of tile frames.

Fourth, `streamserver.py` proxies this tile frame stream via TCP.

Finally, `uncompress` accepts a tile frame stream and displays it via ncurses.

Thus, to stream a game of Dwarf Fortress running on 192.168.0.1 to a viewer on
192.168.0.2, one would use the commands:

On 192.168.0.1:
```sh
cd dfstream && make compressor
dwarffortress &
python2 streamserver.py &
sh x11grab.sh
```

On 192.168.0.2:
```sh
cd dfstream && make uncompress
export LC_CTYPE=en_US.UTF-8
nc 192.168.0.1 8008 | ./uncompress
```

