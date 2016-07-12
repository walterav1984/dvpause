# DVPause

Utility for [DVSwitch](http://dvswitch.alioth.debian.org/wiki/) that allows you to pause videos while feeding them to dvsource-file through a pipe.
This also provides you a nice selection of different locally saved movies. It is supposed to be used for events that require live feeding and pausing with all kinds of locally stored videos.
**However**, yet only PAL is tested. NTSC is supported, but you will propably have to change
```C
#define SEQUENCES 12 // PAL
```
to
```C
#define SEQUENCES 10 // NTSC
```

## Dependencies:
GTK+ (Tested with GTK 3.10, propably 3.6+ supported)
dvsource-file (for make run)

## Compile and Run on ubuntu 16.04:
```
sudo apt-get install libgtk-3-dev libcairo2-dev #ontop of packages to compile dvswitch!!!

cd mygitprojects
git clone https://github.com/walterav1984/dvpause
cd dvpause

make

./main | dvsource-file - -p 1234 -h 127.0.0.1
#only works if dvswitch sub process 'dvsource-file' with "stdin option" was installed see dvswitch!

chmod +x dvpause-full-playlist.sh
./dvpause-full-playlist.sh #edit this file for playlist item / dvswitch location!
```

## Configuration:
The configuration has moved to `dvpause-full-playlist.sh` edit and set location for dvswitch and folder which contains dv files.

##TODO
-[ ] Two sample scenes are provided, but judder so need to reencode original with speedup/pitch fix (24p>25i)
-[ ] Fix arrow key input for skipping/searching in frame steps, forward and backward steps while seeking the start of the clip.
-[ ] CPU usage is very high...
